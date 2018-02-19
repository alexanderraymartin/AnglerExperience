// pair: passthru.vs
// Fast Aproximate Anti-aliasing (FXAA) implementation created using the original 
// 2011 Timothy Lottes Whitepaper https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// and the 2016 Simon Rodriguez tutorial http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
#version 330 core

#define FXAA_EDGE_THRESH (1.0/8.0)
#define FXAA_EDGE_THRESH_MIN (1.0/16.0)
#define FXAA_SUBPIXEL_QUALITY 0.75
#define FXAA_SEARCH_ITERATIONS 12

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

float luma(in vec3 rgb){
	rgb = vec3(0.2126,0.7152, 0.0722)*rgb;
    return(rgb.r+rgb.g+rgb.b);
}

void main()
{	
	vec2 pixsize = 1.0 / resolution;
	vec2 norm_dev_cord = ((FragPos.xy + 1.0) * .5);
	vec4 fullimage = texture(pixtex, norm_dev_cord);
	vec3 image = fullimage.rgb;

	vec3 rgbC  = texture2D(pixtex, norm_dev_cord).rgb; // Center
	vec3 rgbN  = texture2D(pixtex, norm_dev_cord+NORTH).rgb;
	vec3 rgbS  = texture2D(pixtex, norm_dev_cord+SOUTH).rgb;
	vec3 rgbE  = texture2D(pixtex, norm_dev_cord+EAST).rgb;
	vec3 rgbW  = texture2D(pixtex, norm_dev_cord+WEST).rgb;
	vec3 rgbNE = texture2D(pixtex, norm_dev_cord+NORTHEAST).rgb;
	vec3 rgbNW = texture2D(pixtex, norm_dev_cord+NORTHWEST).rgb;
	vec3 rgbSE = texture2D(pixtex, norm_dev_cord+SOUTHEAST).rgb;
	vec3 rgbSW = texture2D(pixtex, norm_dev_cord+SOUTHWEST).rgb;

	float lumaC  = luma(rgbC);
	float lumaN  = luma(rgbN);
	float lumaS  = luma(rgbS);
	float lumaE  = luma(rgbE);
	float lumaW  = luma(rgbW);
	float lumaNE = luma(rgbNE);
	float lumaNW = luma(rgbNW);
	float lumaSE = luma(rgbSE);
	float lumaSW = luma(rgbSW);

	float rangeMin = min(lumaC, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaC, max(max(lumaN, lumaW), max(lumaS, lumaE)));

	float range = rangeMax - rangeMin;
	if( true || range < max(FXAA_EDGE_THRESH_MIN, rangeMax * FXAA_EDGE_THRESH)){
		FragColor = vec4(rgbC, 1.0);
		return;
	}

	vec3 rgbL = (rgbN + rgbW + rgbC + rgbE + rgbS + rgbNW + rgbNE + rgbSW + rgbSE) / 9.0;

	float edgeVert =
		abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
		abs((0.50 * lumaW ) + (-1.0 * lumaC) + (0.50 * lumaE )) +
		abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
	float edgeHorz =
		abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
		abs((0.50 * lumaN ) + (-1.0 * lumaC) + (0.50 * lumaS )) +
		abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
	bool horzSpan = edgeHorz >= edgeVert;

	float luma1 = (horzSpan ? lumaS : lumaW);
	float luma2 = (horzSpan ? lumaN : lumaE);
	float gradientSorW = luma1 - lumaC;
	float gradientNorE = luma2 - lumaC; 

	bool steepest = abs(gradientSorW) >= abs(gradientNorE);

	float gradientScaled = .25*max(abs(gradientSorW), abs(gradientNorE));

	float stepLen = horzSpan ? pixsize.y : pixsize.x;
	float lumaLocal = 0.0;

	if(steepest){
		stepLen = -stepLen;
		lumaLocal = .5*(luma1 + lumaC);
	}else{
		lumaLocal = .5*(luma2 + lumaC);
	}

	vec2 subPixCoord = norm_dev_cord;
	if(horzSpan){
		subPixCoord.y += stepLen * .5;
	}else{
		subPixCoord.x += stepLen *.5;
	}

	vec2 searchOffset = horzSpan ? WEST : NORTH;
	vec2 posSearch = subPixCoord + searchOffset;
	vec2 negSearch = subPixCoord - searchOffset;

	float posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
	float negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

	// True if this is the endpoint along the line
	bool posCap = abs(posLumaEnd) >= gradientScaled;
	bool negCap = abs(negLumaEnd) >= gradientScaled;

	if(!posCap || !negCap){
		for(int i = 1; i < FXAA_SEARCH_ITERATIONS; i++){

			float posLumaEnd = luma(texture(pixtex, posSearch).rgb) - lumaLocal;
			float negLumaEnd = luma(texture(pixtex, negSearch).rgb) - lumaLocal;

			posCap = abs(posLumaEnd) >= gradientScaled;
			negCap = abs(negLumaEnd) >= gradientScaled;


			if(!posCap){
				posSearch += searchOffset;
			}

			if(!negCap){
				negSearch -= searchOffset; 
			}

			if(posCap && negCap){break;}
		}
	}

	float posDistance = horzSpan ? (posSearch.x - norm_dev_cord.x) : (posSearch.y - norm_dev_cord.y);
	float negDistance = horzSpan ? (norm_dev_cord.x - negSearch.x) : (norm_dev_cord.y - negSearch.y);

	float biasNeg = step(negDistance, posDistance);
	float distanceFinal = min(negDistance, posDistance);

	float edgeThickness = (posDistance + negDistance);
	float edgePixelOffset = -distanceFinal / edgeThickness + .5;

	// Detect overshooting during search. 1.0 if correct 0.0 otherwise
	float correctVariation = abs(step(negLumaEnd*biasNeg + posLumaEnd*INV(biasNeg), 0.0) - step(lumaC, lumaLocal));

	float lumaAvg = (1.0/12.0) * (2.0 * (lumaN+lumaS+lumaE+lumaW) + (lumaNE+lumaNW+lumaSE+lumaSW));
	float subPixelOffset1 = clamp(abs(lumaAvg - lumaC)/range, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * FXAA_SUBPIXEL_QUALITY;

	float finalOffset = max(edgePixelOffset*correctVariation, subPixelOffsetFinal);

	vec2 finalCoord;
	if(horzSpan){
		finalCoord.y = norm_dev_cord.y + finalOffset * stepLen;
	}else{
		finalCoord.x = norm_dev_cord.x + finalOffset * stepLen;
	}

	FragColor = texture(pixtex, finalCoord);
}
