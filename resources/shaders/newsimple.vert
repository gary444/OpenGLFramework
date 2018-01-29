#version 150

#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform mat4 DepthBiasMVP;
uniform mat4 MVP;

uniform vec3 LightPosition;

out vec3 pass_Normal;
out vec3 pass_VertexViewPosition;
out vec3 pass_LightSourceViewPosition;
out vec4 pass_ShadowCoord;
out vec2 pass_TexCoord;
//out float pass_test;

void main(void)
{
    
    //define position from MVP matrix
    gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);

    pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;
    
    //for blinnphong shading
    pass_VertexViewPosition = vec3((ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0));
    pass_LightSourceViewPosition = LightPosition;
    
    //textures only for boxes - take form co-ords
    pass_TexCoord = vec2(in_Position.x, in_Position.z);
    
    //position of vertex in light space
    pass_ShadowCoord = DepthBiasMVP * vec4(in_Position,1);
    
//    pass_test = in_Position.z;

    
}



