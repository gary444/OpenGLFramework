#version 150



//in  vec3 pass_Colour;
in vec2 pass_TexCoord;
in vec4 gl_FragCoord;

uniform sampler2D TexID;

out vec4 out_Color;

void main() {
        
    vec4 Col = texture(TexID, pass_TexCoord);
    
    out_Color = vec4(Col);
    
//    out_Color = vec4(1,1,0,1);
}
