//
//  main.cpp
//  rendering_test
//
//  Created by Kirisame Marisa on 16/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#include <iostream>
#include "base.hpp"

class game: public base {
  shader_t test;
  glm::mat4 m;
  
public:
  void init() {
    assets = "/Users/roryb/git/quick_gl/res";
    
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    m = glm::mat4();
    
    load(test, assets / "test.vert.glsl", assets / "test.frag.glsl");
  }
  
  void tick() {
    if (keyboard.is_key_down(KEY_W))
      move(camera, FORWARD);
    if (keyboard.is_key_down(KEY_A))
      move(camera, LEFT);
    if (keyboard.is_key_down(KEY_S))
      move(camera, BACK);
    if (keyboard.is_key_down(KEY_D))
      move(camera, RIGHT);
    if (keyboard.is_key_down(KEY_Q))
      move(camera, UP);
    if (keyboard.is_key_down(KEY_E))
      move(camera, DOWN);
    
    if (keyboard.is_key_down(KEY_B))
      std::cout << "BREAK" << std::endl;
    
    look(camera, mouse.rel());
    
    // m = glm::rotate(m, glm::radians(5.f), glm::vec3(0.f, 1.f, 0.f));
  }
  
  void render(const float& interpol) {
    use(test, [&]() {
      set_uniform("projection", perspective.mat);
      set_uniform("view", camera.mat);
      set_uniform("model", m);
    });
  }
};

START_GAME(game /*, 640, 480, WINDOW_FLAG::RESIZABLE*/)
