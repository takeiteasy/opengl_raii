//
//  buffer.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 19/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef buffer_h
#define buffer_h

#include <vector>
#include "gl.hpp"

namespace helper {
  namespace gl {
    void delete_vertex_array(GLuint id) {
      std::cout << "[LOG] Releasing VAO #" << id << std::endl;
      glDeleteVertexArrays(1, &id);
    }
    
    void delete_buffer_array(GLuint id) {
      std::cout << "[LOG] Releasing VBO #" << id << std::endl;
      glDeleteBuffers(1, &id);
    }
  }
}

struct vertex_array_t: public helper::gl::ptr_t<std::unique_ptr<GLuint, helper::gl::ptr_deleter_t<helper::gl::delete_vertex_array>>> {
  int length;
};

typedef helper::gl::ptr_t<std::unique_ptr<GLuint, helper::gl::ptr_deleter_t<helper::gl::delete_buffer_array>>> buffer_t;
typedef buffer_t render_buffer_t;
typedef buffer_t frame_buffer_t;

auto generate_vertex_array(vertex_array_t& vao) {
  GLuint id;
  glGenVertexArrays(1, &id);
  vao(id);
}

auto bind(vertex_array_t& vao, const std::function<void()>& fn) {
  glBindVertexArray(vao);
  fn();
  glBindVertexArray(0);
}

auto generate_buffer(buffer_t& vbo, void(*genFunc)(GLsizei, GLuint*)) {
  GLuint id;
  genFunc(1, &id);
  vbo(id);
}

static auto generate_array_buffer  = std::bind(generate_buffer, std::placeholders::_1, glGenBuffers);
static auto generate_frame_buffer  = std::bind(generate_buffer, std::placeholders::_1, glGenFramebuffers);
static auto generate_render_buffer = std::bind(generate_buffer, std::placeholders::_1, glGenRenderbuffers);

template<GLenum TYPE> auto bind(buffer_t& vbo) {
  glBindBuffer(TYPE, vbo);
}

template<typename T, GLenum TYPE> auto bind_data(buffer_t& vbo, const std::vector<T>& data, size_t size = 0) {
  bind<TYPE>(vbo);
  glBufferData(TYPE, data.size() * sizeof(T) * (size ? size : 1), &data[0], GL_STATIC_DRAW);
}

template<typename T, GLenum TYPE> auto bind_sub_data(buffer_t& vbo, const std::vector<T>& data, size_t size = 0) {
  bind<TYPE>(vbo);
  glBufferSubData(TYPE, 0, data.size() * sizeof(T) * (size ? size : 1), &data[0]);
}

#endif /* buffer_h */
