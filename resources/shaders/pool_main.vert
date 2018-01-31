#version 150

#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_Texcoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
//uniform mat4 MVP;
uniform mat4 NormalMatrix;
uniform mat4 DepthBiasMVP1;
uniform mat4 DepthBiasMVP2;
uniform int UseModelUV;

out vec3 pass_Normal;
out vec3 pass_VertexViewPosition;
out vec4 pass_ShadowCoord1;
out vec4 pass_ShadowCoord2;
out vec2 pass_TexCoord;

void main(void)
{
    
    //define position from MVP matrix
    gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);

    pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;
    
    //for blinnphong shading
    pass_VertexViewPosition = vec3((ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0));
    
    //if using model UVs - pass through to frag shader
    if (UseModelUV == 1) {
        pass_TexCoord = in_Texcoord;
    }
    else {
        //textures only for boxes - take form co-ords
        pass_TexCoord = vec2(in_Position.x, in_Position.z);
    }
    
    //position of vertex in light spaces
    pass_ShadowCoord1 = DepthBiasMVP1 * vec4(in_Position,1);
    pass_ShadowCoord2 = DepthBiasMVP2 * vec4(in_Position,1);
    
}



