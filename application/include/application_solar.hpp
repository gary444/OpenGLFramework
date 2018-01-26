#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

#define NUM_SPHERES 10

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);

  // draw all objects
  void render() const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void updateView();

    // cpu representation of model
    model_object sphere_object;
    model_object box_object;
    model_object screenquad_object;
    
private:
    //drawing functions
    void uploadAllBoxes(bool shadows) const;
    void uploadBox(box boxToUpload, bool shadows) const;
    void uploadSpheres(bool shadows) const;
    void uploadQuad() const;
    //shadow
    void setupShadowBuffer();
    void setupTestTexture();
//    void renderShadowBuffer();
    
    //mouse varialbe
    bool mouseActive = true;
    float mouseX = 0;
    float mouseY = 0;
    float zoom = 30;
    float slide = 0;
    
    //texture
    GLuint quadTexture;
    
    //light
    glm::fvec3 lightPosition = {-5.0, 5.0, 0.0};
    float visBoxSize = 10;
    
    //shadow
    GLuint fbo_handle;
    GLuint depthTexture;
    
    glm::mat4 biasMatrix = {
                         0.5, 0.0, 0.0, 0.0,
                         0.0, 0.5, 0.0, 0.0,
                         0.0, 0.0, 0.5, 0.0,
                         0.5, 0.5, 0.5, 1.0};
    
    //shapes
    //spheres============================================
    //colour, radius, position
    //parameters
    double triangleSide = 2.0;
    double sphereSize = 0.25;
    int rows = 4;
    double frontX = 3.4;
    glm::vec3 yellow = {0.960, 0.886, 0};
    glm::vec3 red = {1.0, 0.0, 0.0};
    glm::vec3 black = {0.0, 0.0, 0.0};
    
    
    //dependents
    double backX = frontX + (triangleSide * 0.866);//sin 60
    double XperRow = (backX - frontX) / (double)(rows-1);
    double leftZ = triangleSide / 2.0;
    double rightZ = -leftZ;
    double Zadjust = triangleSide / (double)(rows-1);
    
    sphere spheres[NUM_SPHERES] = {{yellow, sphereSize, {frontX, sphereSize, 0.0}},//1 row
        {red, sphereSize, {frontX + XperRow, sphereSize, Zadjust/2.0}},// 2 row
        {yellow, sphereSize, {frontX + XperRow, sphereSize, -Zadjust/2.0}},// 2 row
        {yellow, sphereSize, {backX - XperRow, sphereSize, Zadjust}},//3 row L
        {black, sphereSize, {backX - XperRow, sphereSize, 0.0}},//3 row
        {red, sphereSize, {backX - XperRow, sphereSize, -Zadjust}},//3 row R
        {red, sphereSize, {backX, sphereSize, leftZ}},//4 row L
        {yellow, sphereSize, {backX, sphereSize, leftZ-Zadjust}},//4 row
        {red, sphereSize, {backX, sphereSize, rightZ+Zadjust}},//4 row
        {yellow, sphereSize, {backX, sphereSize, rightZ}}};//4 row R
    
    
    //boxes============================================
    //colour, scaling, position
    
    double cubeSize = 4.0;
    box tower1 = {{0.541, 0.886, 0.917}, {cubeSize / 4.0, cubeSize, cubeSize / 4.0}, {-3.0, 0.0, 0.0}};
    box tower2 = {{0.541, 0.886, 0.917}, {cubeSize / 4.0, cubeSize, cubeSize / 4.0}, {3.0, 0.0, -1.0}};
    
    double tableLength = 18.0;
    double tableWidth = 9.0;
    double tableDepth = 0.2;
    box plane = {{0.039, 0.424, 0.012},
        {tableLength, tableDepth, tableWidth},
        {-(tableLength / 2.0), -tableDepth, -(tableWidth / 2.0)}};
    
    //screen quad - shadow monitor
    std::vector< float >  screenQuad = {-1.0, -1.0, 0.0,
        1.0, -1.0, 0.0,
        -1.0, 1.0, 0.0,
        1.0, 1.0, 0.0};
};

#endif
