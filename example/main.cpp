// Example adapted from https://learnopengl.com/Getting-started/Hello-Triangle
// Just to show that the wrapper works. Relies on GLFW and GLAD.

#include "glad.h"
#include "../gl.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

template<GLenum T> class vertex_buffer_t: public gl::vertex_buffer_t {
public:
  void generate() {
    GLuint id;
    glGenBuffers(1, &id);
    set(id);
  }
  
  void bind() {
    glBindBuffer(T, *this);
  }
};

class vertex_array_t: public gl::vertex_array_t {
public:
  void generate() {
    GLuint id;
    glGenVertexArrays(1, &id);
    set(id);
  }
  
  void bind() {
    glBindVertexArray(*this);
  }
};

class shader_t: public gl::shader_t {
  GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      return -1;
    }
    return shader;
  }
  
public:
  bool create(const char* vertex, const char* fragment) {
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertex);
    if (vertexShader == -1)
      return false;
    GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragment);
    if (fragmentShader == -1)
      return false;
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
      return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    set(shaderProgram);
    return true;
  }
  
  void use() {
    glUseProgram(*this);
  }
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main() {\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main(void) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  
  shader_t shaderProgram;
  if (!shaderProgram.create(vertexShaderSource, fragmentShaderSource))
    return -1;
  
  float vertices[] = {
     0.5f,  0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
  };
  unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
  };
  
  vertex_buffer_t<GL_ARRAY_BUFFER> VBO;
  vertex_buffer_t<GL_ELEMENT_ARRAY_BUFFER> EBO;
  vertex_array_t VAO;
  VAO.generate();
  VBO.generate();
  EBO.generate();
  VAO.bind();
  VBO.bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  EBO.bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    shaderProgram.use();
    // glUseProgram(shaderProgram); // works too
    VAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
