#version 150


in vec3 pass_Normal;
in vec3 pass_VertexViewPosition;
in vec3 pass_LightSourceViewPosition;
in vec4 pass_ShadowCoord;
in vec2 pass_TexCoord;

//in float pass_test;

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

//PCF sampling setup ---------------------------------------
//sun and black cat: http://www.sunandblackcat.com/tipFullView.php?l=eng&topicid=35
const int numSamplingPositions = 4;
// offsets for rectangular PCF sampling
vec2 kernel[4] = vec2[]
(
 vec2(1.0, 1.0), vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0)
 );
// performs sampling and accumulates shadowing factor
void sample(in vec3 coords, in vec2 offset,
            inout float factor, inout float numSamplesUsed)
{
    factor += texture(
                      ShadowMap,
                      vec3(coords.xy + offset, coords.z)
                      );
    numSamplesUsed += 1;
}


void main() {
    
    //material properties
    ambientK = MaterialProperties.x;
    diffuseK = MaterialProperties.y;
    specularK = MaterialProperties.z;
    glossiness = MaterialProperties.w;
    
    //sample texture if needed
    vec3 baseColor = DiffuseColour;
    if (useTexture == 1.0) {
        baseColor = vec3(texture(ColourTex, pass_TexCoord));
    }
    
    
    //shadow mapping========================================
    
    //my current version
//    float bias = 0.1;
//    float distFromLight = texture( ShadowMap, pass_ShadowCoord.xyz );
//    if (distFromLight < (pass_ShadowCoord.z)){
//        visibility = 0.3;
//    }
    
    //ncl demo
    float visibility = 1.0;
//    if(pass_ShadowCoord.w > 0.0) {
//        visibility = textureProj (ShadowMap,pass_ShadowCoord);
//    }

    // sample each PCF texel around current fragment
    float shadowFactor = 0.0;
    float numSamplesUsed = 0.0;
    float PCFRadius = 1.0;
    float shadowMapStep = 1.0 / 1024.0;//1024 is resolution of depth texture
    for(int i=0; i<numSamplingPositions; i++)
    {
        sample(vec3(pass_ShadowCoord),
               kernel[i] * PCFRadius * shadowMapStep,
               shadowFactor,
               numSamplesUsed
               );
    }
    
    // normalize shadowing factor
    visibility = shadowFactor/numSamplesUsed;

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
