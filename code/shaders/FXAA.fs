// PAIR: vert_passthru

// Fast Aproximate Anti-aliasing (FXAA) implementation created using the original 
// 2011 Timothy Lottes Whitepaper https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// and the 2016 Simon Rodriguez tutorial http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
#version 330 core

#define FXAA_EDGE_THRESH (1.0/8.0)
#define FXAA_EDGE_THRESH_MIN (1.0/32.0)
#define FXAA_SUBPIXEL_QUALITY .75
#define FXAA_SEARCH_ITERATIONS 32

#define NORTH ivec2(0.0, 1)
#define SOUTH ivec2(0.0, -1)
#define EAST ivec2(1, 0.0)
#define WEST ivec2(-1, 0.0)
#define NORTHEAST ivec2(1, 1)
#define NORTHWEST ivec2(-1, 1)
#define SOUTHEAST ivec2(1, -1)
#define SOUTHWEST ivec2(-1, -1)

#define INV(_F) (1.0 - (_F))

in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D pixtex;
uniform vec2 resolution;

float luma(in vec3 rgb){
    return(sqrt(dot(rgb, vec3(0.2126,0.7152, 0.0722))));
}

void main()
{	
	vec2 pixsize = 1.0 / resolution;
	vec2 NDC = ((FragPos.xy + 1.0) * .5);

	vec4 fullimage = texture(pixtex, NDC);
	vec3 image = fullimage.rgb;

	vec3 rgbC  = texture2D(pixtex, NDC).rgb; // Center

	float lumaC  = luma(rgbC);
	float lumaN  = luma(textureOffset(pixtex, NDC, NORTH).rgb);
	float lumaS  = luma(textureOffset(pixtex, NDC, SOUTH).rgb);
	float lumaE  = luma(textureOffset(pixtex, NDC, EAST).rgb);
	float lumaW  = luma(textureOffset(pixtex, NDC, WEST).rgb);
	float lumaNE = luma(textureOffset(pixtex, NDC, NORTHEAST).rgb);
	float lumaNW = luma(textureOffset(pixtex, NDC, NORTHWEST).rgb);
	float lumaSE = luma(textureOffset(pixtex, NDC, SOUTHEAST).rgb);
	float lumaSW = luma(textureOffset(pixtex, NDC, SOUTHWEST).rgb);

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
	float horzSpanf = step(edgeVert, edgeHorz);

	float negLuma = horzSpanf*lumaS + INV(horzSpanf)*lumaW;
	float posLuma = horzSpanf*lumaN + INV(horzSpanf)*lumaE;
	float negGradient = negLuma - lumaC;
	float posGradient = posLuma - lumaC; 

	float isNegative = step(abs(negGradient), abs(posGradient));

	float gradientScaled = .25*max(abs(negGradient), abs(posGradient));

	float stepLen = (horzSpanf*pixsize.y + INV(horzSpanf)*pixsize.x) * (-1.0*INV(isNegative) + 1.0*isNegative);
	
	float lumaLocal = .5*(isNegative*posLuma + INV(isNegative)*negLuma + lumaC);


	vec2 searchCoord = NDC + ((stepLen * .5) * mix(vec2(1.0, 0.0), vec2(0.0, 1.0), horzSpanf));

	vec2 searchOffset = INV(horzSpanf)*pixsize.y*NORTH + horzSpanf*pixsize.x*EAST;
	vec2 posSearch = searchCoord + searchOffset;
	vec2 negSearch = searchCoord - searchOffset;

	float posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
	float negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

	float posCap = step(gradientScaled, abs(posLumaEnd));
	float negCap = step(gradientScaled, abs(negLumaEnd));

	if(posCap == 0.0 || negCap == 0.0){
		for(int i = 1; i < FXAA_SEARCH_ITERATIONS; i++){

			posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
			negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

			posCap = step(gradientScaled, abs(posLumaEnd));
			negCap = step(gradientScaled, abs(negLumaEnd));

			// If both ends reached stop iteration
			if((posCap * negCap) == 1.0){break;}

			posSearch += searchOffset*INV(posCap);
			negSearch -= searchOffset*INV(negCap); 
		}
	}

	float posDistance = horzSpanf*(posSearch.x - NDC.x) + INV(horzSpanf)*(posSearch.y - NDC.y);
	float negDistance = horzSpanf*(NDC.x - negSearch.x) + INV(horzSpanf)*(NDC.y - negSearch.y);

	float biasNeg = step(negDistance, posDistance);
	float distanceFinal = min(negDistance, posDistance);

	float edgeLength = (posDistance + negDistance);
	float edgePixelOffset = ((-distanceFinal) / edgeLength + .5);

	// Detect overshooting during search. 1.0 if correct 0.0 otherwise
	float correctVariation = abs(step(negLumaEnd*biasNeg + posLumaEnd*INV(biasNeg), 0.0) - step(lumaC, lumaLocal));

	float lumaAvg = (1.0/12.0) * (2.0 * (lumaN+lumaS+lumaE+lumaW) + (lumaNE+lumaNW+lumaSE+lumaSW));
	float subPixelOffset1 = clamp(abs(lumaAvg - lumaC)/range, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * FXAA_SUBPIXEL_QUALITY;

	float finalOffset = max(edgePixelOffset*correctVariation, subPixelOffsetFinal);

	vec2 finalCoord = horzSpanf*vec2(NDC.x, NDC.y + finalOffset*stepLen) + INV(horzSpanf)*vec2(NDC.x + finalOffset*stepLen, NDC.y);

	FragColor = texture(pixtex, finalCoord);
}
