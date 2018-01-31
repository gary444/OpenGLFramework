#version 150

in vec3 pass_Normal;
in vec3 pass_VertexViewPosition;
in vec4 pass_ShadowCoord1;
in vec4 pass_ShadowCoord2;
in vec2 pass_TexCoord;


uniform vec3 LightPosition1;
uniform vec3 LightPosition2;
uniform vec3 DiffuseColour;
uniform sampler2DShadow ShadowMap1;
uniform sampler2DShadow ShadowMap2;
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

//PCF sampling setup -----------------------------------------------------
//adapted from sun and black cat tutorial
const int numSamplingPositions = 4;
// offsets for rectangular PCF sampling
vec2 kernel[4] = vec2[] (vec2(1.0, 1.0), vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0));
// performs sampling and accumulates shadowing factor
void sample(in vec3 coords, in vec2 offset,
            inout float factor, inout float numSamplesUsed, in sampler2DShadow shadowMap)
{
    factor += texture(shadowMap, vec3(coords.xy + offset, coords.z));
    numSamplesUsed += 1;
}
//light intensity factors
float light1factor = 0.6;
float light2factor = 0.2;


//blinn phong illumination function ------------------------------------------

vec3 calcLighting(in vec3 baseColour, in vec3 inNormal, in vec3 lightPos, in vec3 vertexPos, float visibility) {
    
    vec3 normal = normalize(inNormal);
    
    //create vector for direction of 'light' - from origin to vertex positions in view space
    vec3 lightDir = normalize(lightPos - vertexPos);
    vec3 viewDir = normalize(-vertexPos);
    
    //calculate ambient light
    vec3 ambient = ambientK * baseColour;
    
    //calculate diffuse light
    //lambertian is cos of angle between light and normal
    float lambertian = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lambertian * baseColour * diffuseK;
    
    //specular
    float specularIntensity = 0.0;
    
    vec3 halfwayVector = normalize(viewDir + lightDir);
    float specAngle = max(dot(halfwayVector, normal), 0.0);
    specularIntensity = pow(specAngle, glossiness);
    
    vec3 specular = specularK * specularColour * specularIntensity;
    
    //combine specular, diffuse and ambient
    return ambient + (diffuse * visibility) + (specular * visibility);
}
//end blinn phong function -----------------------------------------------------

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
    
    
    //shadow sampling ========================================
    float visibility = 1.0;

    // sample each PCF texel around current fragment
    float shadowFactor = 0.0;
    float numSamplesUsed = 0.0;
    float PCFRadius = 1.0;
    float shadowMapStep = 1.0 / 1024.0;//1024 is resolution of depth texture
    
    //light1
    for(int i=0; i<numSamplingPositions; i++)
    {
        sample(vec3(pass_ShadowCoord1),
               kernel[i] * PCFRadius * shadowMapStep,
               shadowFactor,
               numSamplesUsed,
               ShadowMap1
               );
    }
    // normalize shadowing factor
    visibility = (shadowFactor/numSamplesUsed) + (1.0-light1factor);
    
    //light2
    shadowFactor = 0.0;
    numSamplesUsed = 0.0;
    for(int i=0; i<numSamplingPositions; i++)
    {
        sample(vec3(pass_ShadowCoord2),
               kernel[i] * PCFRadius * shadowMapStep,
               shadowFactor,
               numSamplesUsed,
               ShadowMap2
               );
    }
    // normalize shadowing factor, add to current shadow
    visibility = (shadowFactor/numSamplesUsed + (1.0-light2factor)) * visibility;


    //end shadow sampling========================================
    
    //lighting calculations for each light
    vec3 light1illumination = calcLighting(baseColor, pass_Normal, LightPosition1, pass_VertexViewPosition, visibility);
    
    vec3 light2illumination = calcLighting(baseColor, pass_Normal, LightPosition2, pass_VertexViewPosition, visibility);
    
    
    //sum and weight light outputs
    out_Color = vec4(
                     (light1factor * light1illumination) +
                     (light2factor * light2illumination), 1.0);

    
}
