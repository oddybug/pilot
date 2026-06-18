#include <Renderer.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Debug.hpp"

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::render_frame() {}

Shader::Shader(const char* name, const char* vertexPath,
               const char* fragmentPath) {
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  // 1. Open files and check for success manually
  vShaderFile.open(vertexPath);
  fShaderFile.open(fragmentPath);

  if (!vShaderFile.is_open()) {
    error("Vertex file not found: ", fragmentPath);
    return;
  }
  if (!fShaderFile.is_open()) {
    error("Fragment file not found: ", fragmentPath);
    return;
  }

  std::stringstream vShaderStream, fShaderStream;
  vShaderStream << vShaderFile.rdbuf();
  fShaderStream << fShaderFile.rdbuf();

  vShaderFile.close();
  fShaderFile.close();

  vertexCode = vShaderStream.str();
  fragmentCode = fShaderStream.str();

  const char* vShaderCode = vertexCode.c_str();
  const char* fShaderCode = fragmentCode.c_str();

  // 2. Compile shaders
  GLuint vertex,
      fragment;  // Changed 'uint' to 'GLuint' for better GLAD compatibility
  int success;
  char infoLog[512];

  // Vertex Shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    error("Vertex shader compilation failed [", name, "]: ", infoLog);
  }

  // Fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    error("Fragment shader compilation failed [", name, "]: ", infoLog);
  }

  // Shader Program
  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    error("Shader linking failed [", name, "]: ", infoLog);
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

void Shader::use() { glUseProgram(ID); }

void Shader::setBool(const char* name, bool value) const {
  glUniform1i(glGetUniformLocation(ID, name), (int)value);
}

void Shader::setInt(const char* name, int value) const {
  glUniform1i(glGetUniformLocation(ID, name), value);
}

void Shader::setFloat(const char* name, float value) const {
  glUniform1f(glGetUniformLocation(ID, name), value);
}

void Shader::setVec2(const char* name, glm::vec2 value) const {
  glUniform2f(glGetUniformLocation(ID, name), value.x, value.y);
}

void Shader::setVec3(const char* name, glm::vec3 value) const {
  glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

void Shader::setMat4(const char* name, const glm::mat4x4& mat) const {
  glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}

Cube::Cube() : position(0.0f), rotation(0.0f), scale(1.0f) {
  float positions[] = {
      // Back face (6 vertices)
      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
      -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      // Front face
      -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
      // Left face
      -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
      // Right face
      0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
      // Bottom face
      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
      0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
      // Top face
      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f};

  float normals[] = {// Back
                     0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
                     0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
                     // Front
                     0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                     // Left
                     -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                     -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                     // Right
                     1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                     0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                     // Bottom
                     0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                     0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                     // Top
                     0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                     1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(2, VBO);

  glBindVertexArray(VAO);

  // Positions
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Cube::~Cube() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(2, VBO);
}

void Cube::draw() const {
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

glm::mat4 Cube::get_model_matrix() const {
  glm::mat4 model = glm::mat4(1.0f);

  // 1. Translation
  model = glm::translate(model, position);

  // 2. Rotation (Order: Z, Y, X is standard to avoid gimbal lock issues)
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

  // 3. Scaling
  model = glm::scale(model, scale);

  return model;
}

Camera::Camera(glm::vec3 startPos, float startFov, float aspect)
    : position(startPos),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      fov(startFov),
      aspectRatio(aspect),
      nearPlane(0.1f),
      farPlane(100.0f) {
  updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + front, up);
}

// Returns the Projection Matrix
glm::mat4 Camera::getProjectionMatrix() const {
  return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::updateCameraVectors() {
  // We assume front is manually adjusted or calculated from Euler angles
  // For a basic perspective camera, we ensure vectors are orthogonal
  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}

void ScreenQuad::draw() {
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

ScreenQuad::ScreenQuad() {
  // Full screen quad: 2 triangles (Position x,y and TexCoords u,v)
  float vertices[] = {// positions   // texCoords
                      -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                      0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                      -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                      1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  // TexCoord attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void*)(2 * sizeof(float)));
}
