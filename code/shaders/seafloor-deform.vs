// PAIR: frag_deferred-export
#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout (location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

uniform float offset;
uniform vec3 groundScale;

out vec3 fragNor;
out vec2 TextureCoordinates;
out vec3 fragPos;


// The MIT License
// Copyright © 2014 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    vec3 hash3( vec2 p )
    {
        vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
               dot(p,vec2(269.5,183.3)), 
               dot(p,vec2(419.2,371.9)) );
      return fract(sin(q)*43758.5453);
    }

    float iqnoise( in vec2 x, float u, float v )
    {
        vec2 p = floor(x);
        vec2 f = fract(x);
        
      float k = 1.0+63.0*pow(1.0-v,4.0);
      
      float va = 0.0;
      float wt = 0.0;
        for( int j=-2; j<=2; j++ )
        for( int i=-2; i<=2; i++ )
        {
            vec2 g = vec2( float(i),float(j) );
        vec3 o = hash3( p + g )*vec3(u,u,1.0);
        vec2 r = g - f + o.xy;
        float d = dot(r,r);
        float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
        va += o.z*ww;
        wt += ww;
        }
      
        return va/wt;
    }
//////////////////////////////////////////////////////////////////////

vec3 displace(in vec3 wpos, in vec2 uv){
  float groundAspect = groundScale.x/groundScale.z;
  vec3 nPos = wpos;
  float distParam = max(0.0, abs(wpos.z - V[3].z) - 5.0);
  uv*=vec2(groundAspect, 1.0); uv.x-=offset*groundAspect; // uv.x-=levelProgress*groundScale.x; 
  distParam = step(1.0, distParam)*pow(distParam, 1.05) + distParam*step(distParam, 1.0);
  nPos += vertNor*(.05*distParam + .15*distParam*smoothstep(0.0, 1.0, iqnoise(uv*10.0, 1.5, 1.0)));
  return(nPos);
}

void main()
{
  fragNor = normalize(transpose(inverse(mat3(M))) * vertNor);
  vec3 worldPos = (M * vec4(vertPos, 1.0)).xyz;

  vec3 newWorldPos = displace(worldPos, vertTex);
  fragPos = newWorldPos;

  gl_Position = P * V * vec4(newWorldPos, 1.0);
  TextureCoordinates = vertTex;
}
