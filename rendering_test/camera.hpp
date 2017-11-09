//
//  camera.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 01/08/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace helper {
  template<typename T> T clamp(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
  }
}

enum VIEW_DIRECTION {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  FORWARD,
  BACK
};

struct projection_t {
  float     fov   = 45.f;
  float     ratio = 1.3333333333f;
  float     near  = 1.f;
  float     far   = 1000.f;
  glm::mat4 mat;
  
  operator glm::mat4() {
    return mat;
  }
};

void update(projection_t& p) {
  p.mat = glm::perspective(p.fov, p.ratio, p.near, p.far);
}

struct camera_t {
  glm::vec3 pos   = {0.f, 0.f,  0.f},
            front = {0.f, 0.f, -1.f},
            up    = {0.f, 1.f,  0.f},
            right,
            world = {0.f, 1.f, 0.f};
  float     yaw         = -90.f,
            pitch       = 0.f,
            sensitivity = .25f,
            move_speed  = 0.05f,
            zoom        = 45.f;
  glm::mat4 mat;
  
  operator glm::mat4() {
    return mat;
  }
};

void update(camera_t& c) {
  c.front = glm::normalize(glm::vec3(cos(glm::radians(c.yaw)) * cos(glm::radians(c.pitch)),
                                     sin(glm::radians(c.pitch)),
                                     sin(glm::radians(c.yaw)) * cos(glm::radians(c.pitch))));
  c.right = glm::normalize(glm::cross(c.front, c.world));
  c.up    = glm::normalize(glm::cross(c.right, c.front));
  
  c.mat = glm::lookAt(c.pos, c.pos + c.front, c.up);
}

void look_at(camera_t& c,
             glm::vec3 _pos   = { .0f, .0f, .0f },
             glm::vec3 _up    = { .0f, 1.f, .0f},
             GLfloat   _yaw   = -90.f,
             GLfloat   _pitch =  0.f) {
  c.pos   = _pos;
  c.up    = _up;
  c.yaw   = _yaw;
  c.pitch = _pitch;
  update(c);
}

void move(camera_t& c, const VIEW_DIRECTION& d) {
  switch (d) {
    case UP:
      c.pos += c.move_speed * c.up;
      break;
    case DOWN:
      c.pos -= c.move_speed * c.up;
      break;
    case LEFT:
      c.pos -= glm::normalize(glm::cross(c.front, c.up)) * c.move_speed;
      break;
    case RIGHT:
      c.pos += glm::normalize(glm::cross(c.front, c.up)) * c.move_speed;
      break;
    case FORWARD:
      c.pos += c.move_speed * c.front;
      break;
    case BACK:
      c.pos -= c.move_speed * c.front;
      break;
  }
}

void look(camera_t& c, const glm::vec2& xy) {
  c.yaw  += xy.x * c.sensitivity;
  c.pitch = helper::clamp(c.pitch + (xy.y * c.sensitivity), -89.0f, 89.0f);
}

#endif /* camera_h */
