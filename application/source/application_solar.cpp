#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

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
,sphere_object{}, box_object{}
{
  initializeGeometry();
  initializeShaderPrograms();
}

void ApplicationSolar::render() const {
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    int i  = 0;
    glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, spheres[i].position);
//    model_matrix = glm::scale(model_matrix, glm::fvec3{spheres[i].radius});
    
    glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));
    
    // bind the VAO to draw
    glBindVertexArray(sphere_object.vertex_AO);
    
    // draw bound vertex array using bound shader
    glDrawElements(sphere_object.draw_mode, sphere_object.num_elements, model::INDEX.type, NULL);
    
//    uploadSpheres(false);
//    uploadAllBoxes(false);
}


void ApplicationSolar::uploadAllBoxes(bool shadows) const{
    
    uploadBox(cube, false);
    uploadBox(plane, false);
}
void ApplicationSolar::uploadBox(box boxToUpload, bool shadows) const{
    
    glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, boxToUpload.position);
    model_matrix = glm::scale(model_matrix, boxToUpload.scaling);
    
    glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
    
    // extra matrix for normal transformation to keep them orthogonal to surface
//    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
//    glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("NormalMatrix"),
//                       1, GL_FALSE, glm::value_ptr(normal_matrix));
//
    //upload colour
    glUniform3fv(m_shaders.at("sphere").u_locs.at("DiffuseColour"), 1, glm::value_ptr(boxToUpload.colour));
    
    // bind the VAO to draw
    glBindVertexArray(box_object.vertex_AO);
    
    // draw bound vertex array using bound shader
    glDrawElements(box_object.draw_mode, box_object.num_elements, model::INDEX.type, NULL);
}

void ApplicationSolar::uploadSpheres(bool shadows) const{
    
    for (int i = 0; i < 1; i++) {
        
        glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, spheres[i].position);
        model_matrix = glm::scale(model_matrix, glm::fvec3{spheres[i].radius});
        
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));
        
        // extra matrix for normal transformation to keep them orthogonal to surface
        glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
        glUniformMatrix4fv(m_shaders.at("sphere").u_locs.at("NormalMatrix"),
                           1, GL_FALSE, glm::value_ptr(normal_matrix));
        
        // bind the VAO to draw
        glBindVertexArray(sphere_object.vertex_AO);
        
        // draw bound vertex array using bound shader
        glDrawElements(sphere_object.draw_mode, sphere_object.num_elements, model::INDEX.type, NULL);
    }
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
}

ApplicationSolar::~ApplicationSolar() {
    
  glDeleteBuffers(1, &box_object.vertex_BO);
  glDeleteVertexArrays(1, &box_object.vertex_AO);
    
    
    glDeleteBuffers(1, &sphere_object.vertex_BO);
    glDeleteBuffers(1, &sphere_object.element_BO);
    glDeleteVertexArrays(1, &sphere_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}
