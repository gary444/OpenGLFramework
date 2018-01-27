#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
,sphere_object{}, box_object{}, screenquad_object{}
{
  initializeGeometry();
  initializeShaderPrograms();
    
    setupShadowBuffer();
    
    setupTestTexture();
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
}

void ApplicationSolar::setupShadowBuffer(){
    
//    fbo_handle = 0;
    glGenFramebuffers(1, &fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
    
    // Depth texture
    //glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glBindTexture ( GL_TEXTURE_2D , 0);
    
    
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    
    glDrawBuffer(GL_NONE); // No color buffer is drawn to.
//    glReadBuffer(GL_NONE);
    
    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::logic_error("framebuffer not correctly initialised");
    
    //needed?
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ApplicationSolar::setupTestTexture(){
    
    pixel_data newTexture = texture_loader::file(m_resource_path + "textures/goat.png");
    
    //get screen size
    GLint viewportData[4];
    glGetIntegerv(GL_VIEWPORT, viewportData);
    
    quadTexture = 3;
    //switch active texture
    glActiveTexture(GL_TEXTURE3);
    //generate texture object
    glGenTextures(1, &quadTexture);
    //bind texture to 2D texture binding point of active unit
    glBindTexture(GL_TEXTURE_2D, quadTexture);
    
    //define sampling parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //add empty texture image
    
    
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportData[2], viewportData[3], 0,
//                 GL_RGB, GL_FLOAT, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTexture.width, newTexture.height, 0, newTexture.channels, newTexture.channel_type, newTexture.ptr());
    

    
}


void ApplicationSolar::render() const {
    
    
    
    
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    
    //1st pass - shadow map ========================================
    glViewport(0,0,1024,1024);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask ( GL_FALSE , GL_FALSE , GL_FALSE , GL_FALSE );
    glUseProgram(m_shaders.at("depth").handle);
    uploadAllBoxes(true);
    
    glColorMask ( GL_TRUE , GL_TRUE , GL_TRUE , GL_TRUE );
    
    //2nd pass - render scene ========================================
    glViewport(0,0,1280,800);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(m_shaders.at("sphere").handle);
    
    //set shadow map as texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(m_shaders.at("sphere").u_locs.at("ShadowMap"), 0);
    
    uploadAllBoxes(false);
    
    
    glViewport(0,0,300,300);
    
    uploadQuad();
}


void ApplicationSolar::uploadAllBoxes(bool shadows) const{
    
    uploadBox(tower1, shadows);
    uploadBox(tower2, shadows);
    uploadBox(plane, shadows);
}
void ApplicationSolar::uploadBox(box boxToUpload, bool shadows) const{
    
    glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, boxToUpload.position);
    model_matrix = glm::scale(model_matrix, boxToUpload.scaling);
    
    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-visBoxSize,visBoxSize,-visBoxSize,visBoxSize,-visBoxSize,visBoxSize);
    glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * model_matrix;
    glm::mat4 depthBiasMVP = biasMatrix * depthMVP;
    if (shadows) {

        //upload depth mvp to depth shader
        glUniformMatrix4fv(m_shaders.at("depth").u_locs.at("DepthMVP"),
                           1, GL_FALSE, &depthMVP[0][0]);
    }
    else {
        
        
        //upload depth MVP with bias to main shader
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("DepthBiasMVP"), 1, GL_FALSE, &depthBiasMVP[0][0]);
        
        glm::fmat4 view_matrix = glm::inverse(m_view_transform);
        
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));
   
        // extra matrix for normal transformation to keep them orthogonal to surface
        glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("NormalMatrix"),
                           1, GL_FALSE, glm::value_ptr(normal_matrix));
        
        //upload colour
        glUniform3fv(m_shaders.at("sphere").u_locs.at("DiffuseColour"), 1, glm::value_ptr(boxToUpload.colour));
        
        //this is to make the sun 'shine' - upload origin with 0.0 as w co-ord
        //    glm::fmat4 view_matrix = glm::inverse(m_view_transform);
        //multiply by view matrx, cast to vec3
        glm::vec3 lightPos_v(view_matrix * glm::vec4(lightPosition, 1.0));
        //upload vec3 to planet shader
        glUniform3fv(m_shaders.at("sphere").u_locs.at("LightPosition"), 1, glm::value_ptr(lightPos_v));
    }
    
    
    
    // bind the VAO to draw
    glBindVertexArray(box_object.vertex_AO);
    
    // draw bound vertex array using bound shader
    glDrawElements(box_object.draw_mode, box_object.num_elements, model::INDEX.type, NULL);
}

void ApplicationSolar::uploadSpheres(bool shadows) const{
    
    for (int i = 0; i < NUM_SPHERES; i++) {
        
        glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, spheres[i].position);
        model_matrix = glm::scale(model_matrix, glm::fvec3{spheres[i].radius});
        
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));
        
        // extra matrix for normal transformation to keep them orthogonal to surface
        glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("NormalMatrix"),
                           1, GL_FALSE, glm::value_ptr(normal_matrix));
        
        glUniform3fv(m_shaders.at("sphere").u_locs.at("DiffuseColour"), 1, glm::value_ptr(spheres[i].colour));

        
        glm::fmat4 view_matrix = glm::inverse(m_view_transform);
        glm::vec3 lightPos(view_matrix * glm::vec4{lightPosition, 1.0});
        glUniform3fv(m_shaders.at("sphere").u_locs.at("LightPosition"), 1, glm::value_ptr(lightPos));
        
        // bind the VAO to draw
        glBindVertexArray(sphere_object.vertex_AO);
        
        // draw bound vertex array using bound shader
        glDrawElements(sphere_object.draw_mode, sphere_object.num_elements, model::INDEX.type, NULL);
    }
}

void ApplicationSolar::uploadQuad() const{
    
    glUseProgram(m_shaders.at("quad").handle);
    //goat = 3
    glUniform1i(m_shaders.at("quad").u_locs.at("TexID"), 0);
    
    glBindVertexArray(screenquad_object.vertex_AO);
    glDrawArrays(screenquad_object.draw_mode, 0, screenquad_object.num_elements);
    
}

void ApplicationSolar::updateView() {
    
    m_view_transform = glm::fmat4{};
    m_view_transform = glm::rotate(m_view_transform, -mouseX, glm::fvec3{0.0, 1.f, 0.0});
    m_view_transform = glm::rotate(m_view_transform, -mouseY, glm::fvec3{1.f, 0.0, 0.0});
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{slide, 0.0, zoom});
    m_view_transform = glm::scale(m_view_transform, glm::fvec3{50.f, 50.f, 50.f});
    
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
    
    glUseProgram(m_shaders.at("sphere").handle);
  glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();
  
  // bind new shader
  glUseProgram(m_shaders.at("sphere").handle);
  
  updateView();
  updateProjection();
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  
    float speed = 0.5;
    
    if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
        zoom -= speed;
        updateView();
    }
    // move scene away from camera
    else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
        zoom += speed;
        updateView();
    }
    // move scene right
    else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
        slide -= speed;
        updateView();
    }
    //move scene left
    else if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
        slide += speed;
        updateView();
    }
}
//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
    
    if (mouseActive) {
        float speed = 0.01f;
        mouseX += pos_x * speed;
        mouseY += pos_y * speed;
        updateView();
    }
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
    m_shaders.emplace("sphere", shader_program{m_resource_path + "shaders/newsimple.vert",
        m_resource_path + "shaders/newsimple.frag"});
  // request uniform locations for shader program
  m_shaders.at("sphere").u_locs["NormalMatrix"] = -1;
  m_shaders.at("sphere").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sphere").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sphere").u_locs["ProjectionMatrix"] = -1;
    m_shaders.at("sphere").u_locs["LightPosition"] = -1;
    m_shaders.at("sphere").u_locs["DiffuseColour"] = -1;
    m_shaders.at("sphere").u_locs["DepthBiasMVP"] = -1;
    m_shaders.at("sphere").u_locs["ShadowMap"] = -1;
    
    m_shaders.emplace("depth", shader_program{m_resource_path + "shaders/depth.vert",
        m_resource_path + "shaders/depth.frag"});
    m_shaders.at("depth").u_locs["DepthMVP"] = -1;
    

    m_shaders.emplace("quad", shader_program{m_resource_path + "shaders/quad.vert",
        m_resource_path + "shaders/quad.frag"});
    m_shaders.at("quad").u_locs["TexID"] = -1;
}

// load models
void ApplicationSolar::initializeGeometry() {
    
    //spheres----------------------------------------------------
    
  model sphere_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &sphere_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(sphere_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &sphere_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, sphere_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sphere_model.data.size(), sphere_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, sphere_model.vertex_bytes, sphere_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, sphere_model.vertex_bytes, sphere_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &sphere_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * sphere_model.indices.size(), sphere_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  sphere_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  sphere_object.num_elements = GLsizei(sphere_model.indices.size());
    
    
    //boxes----------------------------------------------------
    
    model cube_model = model_loader::obj(m_resource_path + "models/cube.obj", model::NORMAL);
    
    // generate vertex array object
    glGenVertexArrays(1, &box_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(box_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &box_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, box_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube_model.data.size(), cube_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, cube_model.vertex_bytes, cube_model.offsets[model::POSITION]);
    
    glEnableVertexAttribArray(1);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, cube_model.vertex_bytes, cube_model.offsets[model::NORMAL]);
    
    // generate generic buffer
    glGenBuffers(1, &box_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * cube_model.indices.size(), cube_model.indices.data(), GL_STATIC_DRAW);
    
    // store type of primitive to draw
    box_object.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object
    box_object.num_elements = GLsizei(cube_model.indices.size());
    
    //======================================================================
    //screen quad
    model screenquad_model = {screenQuad, model::POSITION};

    
    // generate vertex array object
    glGenVertexArrays(1, &screenquad_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(screenquad_object.vertex_AO);
    
    // generate generic buffer
    glGenBuffers(1, &screenquad_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, screenquad_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * screenquad_model.data.size(), screenquad_model.data.data(), GL_STATIC_DRAW);
    
    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, screenquad_model.vertex_bytes, screenquad_model.offsets[model::POSITION]);
    
    
    // transfer number of indices to model object
    screenquad_object.num_elements = GLsizei(screenquad_model.data.size());
    screenquad_object.draw_mode = GL_TRIANGLE_STRIP;
    
}

ApplicationSolar::~ApplicationSolar() {
    
    glDeleteBuffers(1, &box_object.vertex_BO);
    glDeleteBuffers(1, &box_object.element_BO);
    glDeleteVertexArrays(1, &box_object.vertex_AO);
    
    glDeleteFramebuffers(1, &fbo_handle);

    glDeleteBuffers(1, &sphere_object.vertex_BO);
    glDeleteBuffers(1, &sphere_object.element_BO);
    glDeleteVertexArrays(1, &sphere_object.vertex_AO);
    
    glDeleteBuffers(1, &screenquad_object.vertex_BO);
    glDeleteVertexArrays(1, &screenquad_object.vertex_AO);
    
    glDeleteTextures(1, &quadTexture);
    glDeleteTextures(1, &depthTexture);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}
