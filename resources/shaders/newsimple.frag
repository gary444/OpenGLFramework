#version 150
//#version 330


in vec3 pass_Normal;
in vec3 pass_VertexViewPosition;
in vec3 pass_LightSourceViewPosition;
in vec4 pass_ShadowCoord;
in vec2 pass_TexCoord;

uniform vec3 DiffuseColour;
uniform sampler2DShadow ShadowMap;
uniform int useTexture;
uniform sampler2D ColourTex;

//MaterialProperties: {ambientK, diffuseK, specularK, glossiness}
uniform vec4 MaterialProperties;


out vec4 out_Color;

float ambientK;
float diffuseK;
float specularK;
float glossiness;

vec3 specularColour = vec3(1.0, 1.0, 1.0);

void main() {
    
    //material properties
    ambientK = MaterialProperties.x;
    diffuseK = MaterialProperties.y;
    specularK = MaterialProperties.z;
    glossiness = MaterialProperties.w;
    
    //sample texture if needed
    
    vec3 baseColor = DiffuseColour;
    if (useTexture == 1) {
//        baseColor = vec3(texture(ColourTex, pass_TexCoord));
//        out_Color = vec4(1,1,1,1);
    }
//    else {
//        out_Color = vec4(DiffuseColour, 1.0);
//    }
    
    
    //shadow mapping========================================
    
    //my current version
//    float bias = 0.1;
//    float distFromLight = texture( ShadowMap, pass_ShadowCoord.xyz );
//    if (distFromLight < (pass_ShadowCoord.z)){
//        visibility = 0.3;
//    }
    
    //ncl demo
    float visibility = 1.0;
    if(pass_ShadowCoord.w > 0.0) {
        visibility = textureProj (ShadowMap,pass_ShadowCoord);
    }

    
    

    //end shadow mapping========================================
    
    vec3 normal = normalize(pass_Normal);
    
    //create vector for dorection of 'light' - from origin to vertex positions in view space
    vec3 lightDir = normalize(pass_LightSourceViewPosition - pass_VertexViewPosition);
    vec3 viewDir = normalize(-pass_VertexViewPosition);
    
    //calculate ambient light
    vec3 ambient = ambientK * baseColor;
    
    //calculate diffuse light
    //lambertian is cos of angle between light and normal
    float lambertian = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lambertian * baseColor * diffuseK;
    
    //specular
    float specularIntensity = 0.0;
    
    vec3 halfwayVector = normalize(viewDir + lightDir);
    float specAngle = max(dot(halfwayVector, normal), 0.0);
    specularIntensity = pow(specAngle, glossiness);
    
    vec3 specular = specularK * specularColour * specularIntensity;
    
    //combine specular, diffuse and ambient
    out_Color = vec4(ambient + diffuse * visibility + specular * visibility, 1.0);
    
}
