//
//  base.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 18/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef base_hpp
#define base_hpp

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <numeric>

#include "3rdparty/glad.h"
#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_opengl.h>
#include "3rdparty/filesystem.hpp"

#include "inputs.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"

#undef GLAD_DEBUG

enum class CLEAR_BUFFER_BIT: int {
  COLOR   = GL_COLOR_BUFFER_BIT,
  DEPTH   = GL_DEPTH_BUFFER_BIT,
  ACCUM   = GL_ACCUM_BUFFER_BIT,
  STENCIL = GL_STENCIL_BUFFER_BIT,
  ALL     = COLOR bitor DEPTH bitor ACCUM bitor STENCIL
};
  
enum class WINDOW_FLAG: int {
  RESIZABLE   = SDL_WINDOW_RESIZABLE,
  FULLSCREEN = SDL_WINDOW_FULLSCREEN,
  HIDDEN     = SDL_WINDOW_HIDDEN,
  BORDERLESS = SDL_WINDOW_BORDERLESS,
  MAXIMIZED  = SDL_WINDOW_MAXIMIZED,
  MINIMIZED  = SDL_WINDOW_MINIMIZED,
  INPUT_GRAB = SDL_WINDOW_INPUT_GRABBED,
  HIGH_DPI   = SDL_WINDOW_ALLOW_HIGHDPI,
  NONE       = 0
};
  
template <
  unsigned int DEFAULT_WIDTH = 640, unsigned int DEFAULT_HEIGHT = 480,
  WINDOW_FLAG DEFAULT_WINDOW_FLAGS = WINDOW_FLAG::RESIZABLE,
  unsigned int DEFAULT_SWAP_INTERVAL = 1,
  unsigned int DEFAULT_RED_BITS   = 3,  unsigned int DEFAULT_GREEN_BITS   = 3,
  unsigned int DEFAULT_BLUE_BITS  = 2,  unsigned int DEFAULT_ALPHA_BITS   = 0,
  unsigned int DEFAULT_DEPTH_BITS = 16, unsigned     DEFAULT_STENCIL_BITS = 0,
  CLEAR_BUFFER_BIT DEFAULT_CLEAR_BUFFER_BIT = CLEAR_BUFFER_BIT::ALL>
struct configurator {
  static constexpr unsigned int
  default_width      = DEFAULT_WIDTH,      default_height       = DEFAULT_HEIGHT,
  default_red_bits   = DEFAULT_RED_BITS,   default_green_bits   = DEFAULT_GREEN_BITS,
  default_blue_bits  = DEFAULT_BLUE_BITS,  default_alpha_bits   = DEFAULT_ALPHA_BITS,
  default_depth_bits = DEFAULT_DEPTH_BITS, default_stencil_bits = DEFAULT_STENCIL_BITS;
  
  static constexpr unsigned
  default_swap_interval = DEFAULT_SWAP_INTERVAL;
  
  static constexpr CLEAR_BUFFER_BIT
  default_clear_buffer_bit = CLEAR_BUFFER_BIT(DEFAULT_CLEAR_BUFFER_BIT);
  
  static constexpr WINDOW_FLAG
  default_window_flags = WINDOW_FLAG(DEFAULT_WINDOW_FLAGS);
  
  unsigned int
  width      = default_width,      height       = default_height,
  red_bits   = default_red_bits,   green_bits   = default_green_bits,
  blue_bits  = default_blue_bits,  alpha_bits   = default_alpha_bits,
  depth_bits = default_depth_bits, stencil_bits = default_stencil_bits;
  
  int swap_interval = default_swap_interval;
  
  int clear_buffer_bit = int(default_clear_buffer_bit);
  
  int window_flags = int(default_window_flags);
};
  
#ifdef GLAD_DEBUG
void pre_gl_call(const char *name, void *funcptr, int len_args, ...) {
  printf("Calling: %s (%d arguments)\n", name, len_args);
}
#endif
  
static auto glGetError_str(GLenum err) {
  switch (err) {
    case GL_INVALID_ENUM:                  return "INVALID_ENUM"; break;
    case GL_INVALID_VALUE:                 return "INVALID_VALUE"; break;
    case GL_INVALID_OPERATION:             return "INVALID_OPERATION"; break;
    case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW"; break;
    case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION"; break;
    default:
      return "Unknown Error";
  }
}
  
static void post_gl_call(const char *name, void *funcptr, int len_args, ...) {
  GLenum err = glad_glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "ERROR %d (%s) in %s\n", err, glGetError_str(err), name);
    abort();
  }
}
  
static auto init_check(const std::string& msg, bool ok) {
  if (ok)
    std::cout << "[INIT] " << msg << " OK!" << std::endl;
  else {
    std::cout << "[INIT] " << msg << " failed!" << std::endl;
    throw std::runtime_error(SDL_GetError());
  }
}
  
class base {
  std::shared_ptr<std::nullptr_t> sdl, ctx;
  
protected:
  std::shared_ptr<SDL_Window> window;
  int screen_width, screen_height;
  float avg_fps, fps, cur_frame_ticks = .0f;
  path assets = path::getcwd() / "assets";
  bool quit = false;
  
  event_manager    events;
  keyboard_manager keyboard;
  mouse_manager    mouse;
  
  camera_t camera;
  projection_t perspective;
  
  vertex_array_t fbo_quad_vao;
  buffer_t fbo_quad_vbo;
  shader_t fbo_shader;
  frame_buffer_t fbo;
  render_buffer_t rbo;
  texture_t fbo_texture;
  
public:
  virtual void pre_init() {}
  virtual void init() = 0;
  virtual void tick() = 0;
  virtual void render(const float&) = 0;
  
  virtual void post_process() {
    use(fbo_shader, [&]() {
      glBindVertexArray(fbo_quad_vao);
      glBindTexture(GL_TEXTURE_2D, fbo_texture);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    });
  }
  
  template <class C = configurator<>> auto start(const C& c) {
    pre_init();
    
    init_check("SDL_Init()", SDL_Init(SDL_INIT_VIDEO) >= 0);
    sdl = std::shared_ptr<std::nullptr_t>(nullptr, [](std::nullptr_t) {
      SDL_Quit();
    });
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    screen_width  = c.width;
    screen_height = c.height;
    
    look_at(camera, glm::vec3(0.f, 1.5f, 3.f), glm::vec3(0.f, 1.f, 0.f));
    update(perspective);
    
    window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("twwepdnutis",
                                                          SDL_WINDOWPOS_CENTERED,
                                                          SDL_WINDOWPOS_CENTERED,
                                                          screen_width,
                                                          screen_height,
                                                          SDL_WINDOW_OPENGL bitor SDL_WINDOW_SHOWN bitor c.window_flags),
                                         SDL_DestroyWindow);
    init_check("SDL_CreateWindow()", bool(window));
    
    auto __ctx = SDL_GL_CreateContext(window.get());
    ctx = std::shared_ptr<std::nullptr_t>(nullptr, [__ctx](SDL_GLContext _ctx) {
      SDL_GL_DeleteContext(_ctx);
    });
    init_check("SDL_GL_CreateContext()", bool(__ctx));
    
    init_check("gladInit", gladLoadGL());
    
#ifdef GLAD_DEBUG
    glad_set_pre_callback(pre_gl_call);
#endif
    glad_set_post_callback(post_gl_call);
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     c.red_bits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   c.green_bits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    c.blue_bits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   c.alpha_bits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   c.depth_bits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, c.stencil_bits);
    
    events.quit = [&](const SDL_QuitEvent&) {
      quit = true;
    };
    
    events.window_event = [&](const SDL_WindowEvent& e) {
      if (e.event == SDL_WINDOWEVENT_RESIZED) {
        screen_width  = static_cast<float>(e.data1);
        screen_height = static_cast<float>(e.data2);
        perspective.ratio = screen_width / screen_height;
        update(perspective);
        
        glViewport(0, 0, e.data1, e.data2);
      }
    };
    
    keyboard.link_event_manager(events);
    mouse.link_event_manager(events);
    
    std::vector<float> fb_quads_verts = {
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
      
      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f
    };
    
    load(fbo_shader,
         GLSL(330,
              layout (location = 0) in vec2 position;
              layout (location = 1) in vec2 tex_coords;
              
              out vec2 TexCoords;
              
              void main() {
                TexCoords = tex_coords;
                gl_Position = vec4(position.x, position.y, 0.0, 1.0);
              }),
         GLSL(330,
              out vec4 FragColor;
              
              in vec2 TexCoords;
              
              uniform sampler2D iChannel0;
              
              void main() {
                FragColor = vec4(texture(iChannel0, TexCoords).rgb, 1.0);
              }));
    
    generate_frame_buffer(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    generate_texture(fbo_texture);
    bind(fbo_texture, [&]() {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      add_sampleri(GL_TEXTURE_MIN_FILTER, GL_LINEAR,
                   GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
    });
    
    generate_render_buffer(rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_width, screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("ERROR! Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    generate_vertex_array(fbo_quad_vao);
    generate_array_buffer(fbo_quad_vbo);
    
    bind(fbo_quad_vao, [&]() {
      bind_data<float, GL_ARRAY_BUFFER>(fbo_quad_vbo, fb_quads_verts);
      
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    });
    
    init();
    
    constexpr auto max_frame_skip = 10;
    constexpr auto frame_rate     = std::chrono::duration<std::chrono::high_resolution_clock::rep, std::ratio<1, 60>>{1};
    constexpr auto frame_rate_c   = std::chrono::duration_cast<std::chrono::duration<float>>(frame_rate).count();
    auto           next_tick      = std::chrono::high_resolution_clock::now() + frame_rate;
    
    auto last_frame_tick = std::chrono::high_resolution_clock::now();
    std::vector<float> fps_avg_vec;
    fps_avg_vec.reserve(10);
    
    while (not quit) {
      int loops = 0;
      while (std::chrono::high_resolution_clock::now() > next_tick and loops < max_frame_skip) {
        events.poll();
        
        tick();
        
        update(camera);
        mouse.reset();
        
        next_tick += frame_rate;
        loops++;
      }
      
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glEnable(GL_DEPTH_TEST);
      
      glClear(c.clear_buffer_bit);
      
      render(std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() + frame_rate - next_tick).count() / frame_rate_c);
      
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDisable(GL_DEPTH_TEST);
      glClear(GL_COLOR_BUFFER_BIT);
      
      post_process();
      
      SDL_GL_SwapWindow(window.get());
      
      cur_frame_ticks++;
      if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - last_frame_tick).count() >= 1) {
        if (fps_avg_vec.size() == 10)
          fps_avg_vec.erase(fps_avg_vec.begin());
        
        fps = cur_frame_ticks;
        
        fps_avg_vec.push_back(fps);
        avg_fps = std::accumulate(fps_avg_vec.begin(), fps_avg_vec.end(), 0.0f) / fps_avg_vec.size();
        
        last_frame_tick = std::chrono::high_resolution_clock::now();
        cur_frame_ticks = 0;
      }
    }
  }
};

#define START_GAME(x, ...) \
auto main(int argc, const char* argv[]) -> int try { \
  configurator<__VA_ARGS__> c; \
  x().start(c); \
} catch (const std::runtime_error& e) { \
  std::cout << "exception: " << e.what() << std::endl; \
} catch (...) { \
  std::cout << "exception: unknown" << std::endl; \
}
  
#endif /* base_hpp */
