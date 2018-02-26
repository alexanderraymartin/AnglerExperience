// pair: passthru.vs
// Fast Aproximate Anti-aliasing (FXAA) implementation created using the original 
// 2011 Timothy Lottes Whitepaper https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// and the 2016 Simon Rodriguez tutorial http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
#version 330 core

#define FXAA_EDGE_THRESH (1.0/8.0)
#define FXAA_EDGE_THRESH_MIN (1.0/32.0)
#define FXAA_SUBPIXEL_QUALITY .75
#define FXAA_SEARCH_ITERATIONS 128

#define NORTH vec2(0.0, pixsize.y)
#define SOUTH vec2(0.0, -pixsize.y)
#define EAST vec2(pixsize.x, 0.0)
#define WEST vec2(-pixsize.x, 0.0)
#define NORTHEAST vec2(pixsize.x, pixsize.y)
#define NORTHWEST vec2(-pixsize.x, pixsize.y)
#define SOUTHEAST vec2(pixsize.x, -pixsize.y)
#define SOUTHWEST vec2(-pixsize.x, -pixsize.y)

#define INV(_F) (1.0 - (_F))

in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D pixtex;
uniform vec2 resolution;
uniform bool showEdges;
uniform bool showPosNeg;
uniform bool showEarlyCaps;
uniform bool shadeEPO;
uniform bool useFXAA;

float luma(in vec3 rgb){
    return(sqrt(dot(rgb, vec3(0.2126,0.7152, 0.0722))));
}

void main()
{	
	vec2 pixsize = 1.0 / resolution;
	vec2 norm_dev_cord = ((FragPos.xy + 1.0) * .5);
	vec4 fullimage = texture(pixtex, norm_dev_cord);
	vec3 image = fullimage.rgb;

	vec3 rgbC  = texture2D(pixtex, norm_dev_cord).rgb; // Center

	float lumaC  = luma(rgbC);
	float lumaN  = luma(texture2D(pixtex, norm_dev_cord+NORTH).rgb);
	float lumaS  = luma(texture2D(pixtex, norm_dev_cord+SOUTH).rgb);
	float lumaE  = luma(texture2D(pixtex, norm_dev_cord+EAST).rgb);
	float lumaW  = luma(texture2D(pixtex, norm_dev_cord+WEST).rgb);
	float lumaNE = luma(texture2D(pixtex, norm_dev_cord+NORTHEAST).rgb);
	float lumaNW = luma(texture2D(pixtex, norm_dev_cord+NORTHWEST).rgb);
	float lumaSE = luma(texture2D(pixtex, norm_dev_cord+SOUTHEAST).rgb);
	float lumaSW = luma(texture2D(pixtex, norm_dev_cord+SOUTHWEST).rgb);

	float rangeMin = min(lumaC, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaC, max(max(lumaN, lumaW), max(lumaS, lumaE)));

	float range = rangeMax - rangeMin;
	if( range < max(FXAA_EDGE_THRESH_MIN, rangeMax * FXAA_EDGE_THRESH)){
		FragColor = vec4(rgbC, 1.0);
		return;
	}


	float edgeVert =
		abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
		abs((0.50 * lumaW ) + (-1.0 * lumaC) + (0.50 * lumaE )) +
		abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
	float edgeHorz =
		abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
		abs((0.50 * lumaN ) + (-1.0 * lumaC) + (0.50 * lumaS )) +
		abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
	bool horzSpan = edgeHorz >= edgeVert; 
	float horzSpanf = step(edgeVert, edgeHorz);

	if(showEdges){
		FragColor = vec4(( horzSpan ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0) ) , 1.0);
		return;
	}

	float negLuma = (horzSpan ? lumaS : lumaW);
	float posLuma = (horzSpan ? lumaN : lumaE);
	float negGradient = negLuma - lumaC;
	float posGradient = posLuma - lumaC; 

	float isNegative = step(abs(negGradient), abs(posGradient));

	if(showPosNeg){
		FragColor = vec4(vec3(isNegative), 1.0);
		return;
	}

	float gradientScaled = .25*max(abs(negGradient), abs(posGradient));

	float stepLen = (horzSpan ? pixsize.y : pixsize.x) * (-1.0*INV(isNegative) + 1.0*isNegative);
	
	float lumaLocal = .5*(isNegative*posLuma + INV(isNegative)*negLuma + lumaC);


	vec2 searchCoord = norm_dev_cord + ((stepLen * .5) * mix(vec2(1.0, 0.0), vec2(0.0, 1.0), horzSpanf));

	vec2 searchOffset = INV(horzSpanf)*NORTH + horzSpanf*EAST;
	vec2 posSearch = searchCoord + searchOffset;
	vec2 negSearch = searchCoord - searchOffset;

	float posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
	float negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

	// True if this is the endpoint along the line
	bool posCap = abs(posLumaEnd) >= gradientScaled;
	bool negCap = abs(negLumaEnd) >= gradientScaled;

	if(!posCap || !negCap){
		for(int i = 1; i < FXAA_SEARCH_ITERATIONS; i++){

			posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
			negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

			posCap = abs(posLumaEnd) >= gradientScaled;
			negCap = abs(negLumaEnd) >= gradientScaled;

			if(posCap && negCap){break;}

			if(!posCap){
				posSearch += searchOffset;
			}

			if(!negCap){
				negSearch -= searchOffset; 
			}
		}
	}else if(showEarlyCaps){
		FragColor = vec4(vec3(1.0, isNegative, 0.0), 1.0);
		return;
	}

	float posDistance = horzSpan ? (posSearch.x - norm_dev_cord.x) : (posSearch.y - norm_dev_cord.y);
	float negDistance = horzSpan ? (norm_dev_cord.x - negSearch.x) : (norm_dev_cord.y - negSearch.y);

	float biasNeg = step(negDistance, posDistance);
	float distanceFinal = min(negDistance, posDistance);

	float edgeLength = (posDistance + negDistance);
	float edgePixelOffset = ((-distanceFinal) / edgeLength + .5);


	if(shadeEPO){
		// FragColor = vec4(vec3(edgePixelOffset),1.0);
		FragColor = vec4(vec3(smoothstep(0.0, pixsize.x*45.0, distanceFinal), -edgePixelOffset, edgePixelOffset), 1.0);
		return;
	}

	// Detect overshooting during search. 1.0 if correct 0.0 otherwise
	float correctVariation = abs(step(negLumaEnd*biasNeg + posLumaEnd*INV(biasNeg), 0.0) - step(lumaC, lumaLocal));

	float lumaAvg = (1.0/12.0) * (2.0 * (lumaN+lumaS+lumaE+lumaW) + (lumaNE+lumaNW+lumaSE+lumaSW));
	float subPixelOffset1 = clamp(abs(lumaAvg - lumaC)/range, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * FXAA_SUBPIXEL_QUALITY;

	float finalOffset = max(edgePixelOffset*correctVariation, subPixelOffsetFinal);

	vec2 finalCoord = norm_dev_cord;
	if(horzSpan){
		finalCoord.y = norm_dev_cord.y + finalOffset * stepLen;
	}else{
		finalCoord.x = norm_dev_cord.x + finalOffset * stepLen;
	}

	FragColor = texture(pixtex, useFXAA ? finalCoord : norm_dev_cord);
}
