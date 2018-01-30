//
//  mouse.hpp
//  twwfpdnutis
//
//  Created by George Watson on 07/05/2017.
//
//

#ifndef mouse_h
#define mouse_h

#include <array>

#include <SDL2/SDL_mouse.h>
#include <glm/vec2.hpp>

#include "inputs.hpp"

enum mousecode_t {
  MOUSE_LEFT	 = SDL_BUTTON_LEFT,
  MOUSE_MIDDLE = SDL_BUTTON_MIDDLE,
  MOUSE_RIGHT  = SDL_BUTTON_RIGHT,
  MOUSE_X1		 = SDL_BUTTON_X1,
  MOUSE_X2		 = SDL_BUTTON_X2,
  MOUSE_LMASK  = SDL_BUTTON_LMASK,
  MOUSE_MMASK  = SDL_BUTTON_MMASK,
  MOUSE_RMASK  = SDL_BUTTON_RMASK,
  MOUSE_X1MASK = SDL_BUTTON_X1MASK,
  MOUSE_X2MASK = SDL_BUTTON_X2MASK
};

class mouse_manager {
  std::array<bool,     10> state		 = { { false } };
  std::array<uint32_t, 10> down_tick = { { 0 } },
                           up_tick	 = { { 0 } };
  glm::vec2 _pos, _rel, _wheel;
  bool inverted_y = false;
  
public:
  auto link_event_manager(event_manager& em) {
    em.mouse_button_down = [&](const SDL_MouseButtonEvent& e) {
      state[e.button] = true;
    };
    
    em.mouse_button_up = [&](const SDL_MouseButtonEvent& e) {
      state[e.button] = false;
    };
    
    em.mouse_motion = [&](const SDL_MouseMotionEvent& e) {
      _pos.x = static_cast<float>(e.x);
      _pos.y = static_cast<float>(e.y);
      _rel.x = static_cast<float>(e.xrel);
      _rel.y = static_cast<float>(inverted_y ? e.yrel : -e.yrel);
    };
    
    em.mouse_wheel = [&](const SDL_MouseWheelEvent& e) {
      _wheel.x = static_cast<float>(e.x);
      _wheel.y = static_cast<float>(e.y);
    };
  }
  
  auto is_mouse_down(const mousecode_t& button) {
    return state[button] == true;
  }
  
  auto is_mouse_up(const mousecode_t& button) {
    return state[button] == false;
  }
  
  auto mouse_down_tick(const mousecode_t& mouse) {
    return down_tick[mouse];
  }
  
  auto mouse_up_tick(const mousecode_t& mouse) {
    return up_tick[mouse];
  }
  
  auto ticks_since_mouse_down(const mousecode_t& mouse) {
    return SDL_GetTicks() - down_tick[mouse];
  }
  
  auto ticks_since_mouse_up(const mousecode_t& mouse) {
    return SDL_GetTicks() - up_tick[mouse];
  }
  
  auto toggle_inverted_y() {
    inverted_y = not inverted_y;
  }
  
  auto set_inverted_y(bool set_to) {
    inverted_y = set_to;
  }
  
  auto is_inverted_y() {
    return inverted_y;
  }
  
  auto& wheel() const {
    return _wheel;
  }
  
  auto& rel() const {
    return _rel;
  }
  
  auto& pos() const {
    return _pos;
  }
  
  auto reset() {
    _wheel = { 0, 0 };
    _rel   = { 0, 0 };
  }
};


#endif /* mouse_h */
