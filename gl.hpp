//
//  gl.hpp
//  opengl_raii
//
//  Created by Rory B. Bellows on 02/04/2020.
//  Copyright © 2020 Rory B. Bellows. All rights reserved.
//

#ifndef gl_hpp
#define gl_hpp

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <memory>

namespace gl {
  namespace helper {
    template<void (*func)(GLuint)> struct ptr_deleter_t {
      typedef GLuint pointer;
      void operator()(GLuint id) { func(id); }
    };
    
    static void delete_vertex_array(GLuint id) {
      glDeleteVertexArrays(1, &id);
    }
    
    static void delete_buffer_array(GLuint id) {
      glDeleteBuffers(1, &id);
    }
    
    static void delete_frame_buffer(GLuint id) {
      glDeleteFramebuffers(1, &id);
    }
    
    static void delete_render_buffer(GLuint id) {
      glDeleteRenderbuffers(1, &id);
    }
    
    static void delete_texture(GLuint id) {
      glDeleteTextures(1, &id);
    }
    
    static void delete_program(GLuint id) {
      glDeleteProgram(id);
    }
  }
  
  template <typename T> class ptr_t  {
    T ptr;
    
  protected:
    void set(GLuint id) {
      ptr.reset();
      ptr = T(id);
    }
    
  public:
    ptr_t<T>() {}
    ptr_t<T>(const ptr_t<T> &id) {
      ptr = std::move(id);
    }
    ptr_t<T>(const GLuint &id) {
      ptr = T(id);
    }
    
    ptr_t<T>& operator =(GLuint id) {
      ptr.reset();
      ptr = T(id);
      return *this;
    }
    
    T& operator ()(GLuint id) {
      ptr.reset();
      ptr = T(id);
      return ptr;
    }
    
    operator GLuint() {
      return ptr.get();
    }
  };
  
  using buffer_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_buffer_array>>>;
  class vertex_buffer_t: public buffer_t{};
  class element_buffer_t: public buffer_t{};
  using frame_buffer_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_frame_buffer>>>;
  using render_buffer_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_render_buffer>>>;
  using vertex_array_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_vertex_array>>>;
  using texture_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_texture>>>;
  using shader_t = ptr_t<std::unique_ptr<GLuint, helper::ptr_deleter_t<helper::delete_program>>>;
}

#endif /* gl_hpp */
