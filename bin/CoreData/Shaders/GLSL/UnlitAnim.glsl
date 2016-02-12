#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Fog.glsl"

varying vec2 vTexCoord;
varying vec4 vWorldPos;
#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif

#ifdef COMPILEVS

uniform float cAnimTime;
uniform float cAnimTileX;
uniform float cAnimTileY;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetTexCoord(iTexCoord);
    vWorldPos = vec4(worldPos, GetDepth(gl_Position));
    
    int index = int(cAnimTime);  
    index = index % int(cAnimTileX * cAnimTileY);
    
    vec2 size = vec2(1.0 / float(cAnimTileX), 1.0 / float(cAnimTileY));
    
    // split into horizontal and vertical index
	int uIndex = index % int((cAnimTileX));
	int vIndex = index / int((cAnimTileY));
    
    
    vec2 finalFrame = iTexCoord;
    finalFrame.x /= cAnimTileX;
    finalFrame.y /= cAnimTileY;
    
    finalFrame += vec2((uIndex) * size.x, (vIndex) * size.y);
     
    vTexCoord = GetTexCoord(finalFrame);
    

    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif
}

#endif

void PS()
{
    // Get material diffuse albedo
    #ifdef DIFFMAP
        vec4 diffColor = cMatDiffColor * texture2D(sDiffMap, vTexCoord);
        #ifdef ALPHAMASK
            if (diffColor.a < 0.5)
                discard;
        #endif
    #else
        vec4 diffColor = cMatDiffColor;
    #endif

    #ifdef VERTEXCOLOR
        diffColor *= vColor;
    #endif

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
    #else
        float fogFactor = GetFogFactor(vWorldPos.w);
    #endif

    #if defined(PREPASS)
        // Fill light pre-pass G-Buffer
        gl_FragData[0] = vec4(0.5, 0.5, 0.5, 1.0);
        gl_FragData[1] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #elif defined(DEFERRED)
        gl_FragData[0] = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
        gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
        gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
        gl_FragData[3] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #else
        gl_FragColor = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
    #endif
}
