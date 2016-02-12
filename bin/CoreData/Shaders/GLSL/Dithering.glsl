#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;

const float RGBSize = 16.0;
const float Divisor = 4.0;



void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

#ifdef COMPILEPS

float quantize(float inp, float period)
{
    return floor((inp+period/2.)/period)*period;
}
vec2 quantize(vec2 inp, vec2 period)
{
    return floor((inp+period/2.)/period)*period;
}

vec3 getSceneColor(vec2 uv)
{
    return texture2D(sDiffMap, uv).rgb;
}

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float bayer4x4(vec2 uvScreenSpace)
{
	vec2 bayerCoord = floor(uvScreenSpace/Divisor);
    bayerCoord = mod(bayerCoord, 4.);
    
    const mat4 bayerMat = mat4(
    1,9,3,11,
    13,5,15,7,
    4,12,2,10,
    16,8,14,6) / 16.;
    
    int bayerIndex = int(bayerCoord.x + bayerCoord.y * 4.);
    if(bayerIndex == 0) return bayerMat[0][0];
    if(bayerIndex == 1) return bayerMat[0][1];
    if(bayerIndex == 2) return bayerMat[0][2];
    if(bayerIndex == 3) return bayerMat[0][3];
    if(bayerIndex == 4) return bayerMat[1][0];
    if(bayerIndex == 5) return bayerMat[1][1];
    if(bayerIndex == 6) return bayerMat[1][2];
    if(bayerIndex == 7) return bayerMat[1][3];
    if(bayerIndex == 8) return bayerMat[2][0];
    if(bayerIndex == 9) return bayerMat[2][1];
    if(bayerIndex == 10) return bayerMat[2][2];
    if(bayerIndex == 11) return bayerMat[2][3];
    if(bayerIndex == 12) return bayerMat[3][0];
    if(bayerIndex == 13) return bayerMat[3][1];
    if(bayerIndex == 14) return bayerMat[3][2];
    if(bayerIndex == 15) return bayerMat[3][3];

    return 10.;// impossible
}
#line 72
void PS()
{
    vec2 fragCoord = gl_FragCoord.xy;
    
    vec2 iResolution = vec2(1.0 / cGBufferInvSize.x, 1.0 / cGBufferInvSize.y);
        
    vec3 quantizationPeriod = vec3(1.0 / (RGBSize - 1.0));
    
    vec2 uvPixellated = floor(fragCoord / Divisor) * Divisor;
    
    vec3 originalCol = getSceneColor(vScreenPos);
    
    vec3 dc = getSceneColor(uvPixellated / iResolution.xy);
    
    dc += (bayer4x4(vScreenPos)-0.5)*(quantizationPeriod);
      
    dc = vec3(
        quantize(dc.r, quantizationPeriod.r),
        quantize(dc.g, quantizationPeriod.g),
        quantize(dc.b, quantizationPeriod.b)
            );
    
    gl_FragColor = vec4(dc, 1.0);
}

#endif

