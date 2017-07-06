#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

// #define NO_GRID
// #define NO_INFO
#include "src/base_game.hpp"
#include "src/texture.hpp"

#pragma clang diagnostic pop

class example_game: public game_t {
  unsigned int VBO, VAO;
  texture_t texture;
  program_t shader;
  glm::mat4 cube_world;
  
public:
  void init() {
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    float vertices[] = {
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
      
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    texture.load(assets / "test.png")
    .mipmap()
    .done();
    
    shader.load().attach(vertex(GLSL(330,
                                     layout (location = 0) in vec3 aPos;
                                     layout (location = 1) in vec2 aTexCoord;
                                     
                                     out vec2 TexCoord;
                                     uniform mat4 model;
                                     uniform mat4 view;
                                     uniform mat4 projection;
                                     
                                     void main() {
                                       gl_Position = projection * view * model * vec4(aPos, 1.0);
                                       TexCoord = vec2(aTexCoord.x, -aTexCoord.y);
                                     })),
                         fragment(GLSL(330,
                                       out vec4 FragColor;
                                       
                                       in vec2 TexCoord;
                                       uniform sampler2D channel0;
                                       
                                       void main() {
                                         FragColor = vec4(texture(channel0, TexCoord).rgb, 1.f);
                                       })))
    .link()
    .done();
    
    cube_world = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 1.0f, 0.0f));
  }
  
  void update() {
    if (keyboard.is_key_down(KEY_ESCAPE))
      quit = true;
    
    if (keyboard.is_key_down(KEY_W))
      view.move(FORWARD);
    if (keyboard.is_key_down(KEY_A))
      view.move(LEFT);
    if (keyboard.is_key_down(KEY_S))
      view.move(BACK);
    if (keyboard.is_key_down(KEY_D))
      view.move(RIGHT);
    if (keyboard.is_key_down(KEY_Q))
      view.move(UP);
    if (keyboard.is_key_down(KEY_E))
      view.move(DOWN);
    
    if (keyboard.is_key_down(KEY_B))
      std::cout << "BREAK" << std::endl;
    
    view.look(mouse.rel());
    
    cube_world = glm::rotate(cube_world, glm::radians(cur_frame_ticks * 0.05f), glm::vec3(0.5f, 1.0f, 0.0f));
  }
  
  void render(const float& interpol) {
    shader.use([&]() {
      texture.bind();
      set_mvp(cube_world, view, proj);
      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glBindVertexArray(0);
      texture.done();
    });
  }
};

START_GAME(example_game /*, 640, 480, WINDOW_FLAG::RESIZABLE*/)
