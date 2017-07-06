//
//  base_game.hpp
//  twwfpdnutis
//
//  Created by George Watson on 04/05/2017.
//
//

#ifndef base_game_h
#define base_game_h

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <numeric>

#include "3rdparty/glad.h"
#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_opengl.h>
#include <boost/filesystem.hpp>

#include "events.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "projection.hpp"
#include "camera.hpp"
#include "shader.hpp"

#ifndef DONT_DEFINE_MACROS
// I like unless, call the care police, because I don't give a fuck
#define unless(x) if (not (x))
#define BENCHMARK_START auto __bench__ = std::chrono::steady_clock::now();
#define BENCHMARK_END   std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - __bench__).count() << "ms" << std::endl;
#endif

enum class CLEAR_BUFFER_BIT: int {
	COLOR   = GL_COLOR_BUFFER_BIT,
	DEPTH   = GL_DEPTH_BUFFER_BIT,
	ACCUM   = GL_ACCUM_BUFFER_BIT,
	STENCIL = GL_STENCIL_BUFFER_BIT,
	ALL			= COLOR bitor DEPTH bitor ACCUM bitor STENCIL
};

enum class WINDOW_FLAG: int {
	RESIZABLE	 = SDL_WINDOW_RESIZABLE,
	FULLSCREEN = SDL_WINDOW_FULLSCREEN,
	HIDDEN		 = SDL_WINDOW_HIDDEN,
	BORDERLESS = SDL_WINDOW_BORDERLESS,
	MAXIMIZED  = SDL_WINDOW_MAXIMIZED,
	MINIMIZED  = SDL_WINDOW_MINIMIZED,
	INPUT_GRAB = SDL_WINDOW_INPUT_GRABBED,
	HIGH_DPI   = SDL_WINDOW_ALLOW_HIGHDPI,
	NONE			 = 0
};

#ifdef GLAD_DEBUG
void pre_gl_call(const char *name, void *funcptr, int len_args, ...) {
	printf("Calling: %s (%d arguments)\n", name, len_args);
}
#endif

char* glGetError_str(GLenum err) {
	switch (err) {
		case GL_INVALID_ENUM:                  return "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 return "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             return "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION"; break;
		default:
																					 return "Unknown Error";
	}
}

void post_gl_call(const char *name, void *funcptr, int len_args, ...) {
	GLenum err = glad_glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "ERROR %d (%s) in %s\n", err, glGetError_str(err), name);
		abort();
	}
}

template <
unsigned int DEFAULT_WIDTH = 640, unsigned int DEFAULT_HEIGHT = 480,
						 WINDOW_FLAG DEFAULT_WINDOW_FLAGS = WINDOW_FLAG::RESIZABLE,
						 unsigned int DEFAULT_SWAP_INTERVAL = 1,
						 unsigned int DEFAULT_RED_BITS   = 3,  unsigned int DEFAULT_GREEN_BITS   = 3,
						 unsigned int DEFAULT_BLUE_BITS  = 2,  unsigned int DEFAULT_ALPHA_BITS   = 0,
						 unsigned int DEFAULT_DEPTH_BITS = 16, unsigned		 DEFAULT_STENCIL_BITS = 0,
						 CLEAR_BUFFER_BIT DEFAULT_CLEAR_BUFFER_BIT = CLEAR_BUFFER_BIT::ALL>
						 struct configurator {
							 static constexpr unsigned int
								 default_width      = DEFAULT_WIDTH,			 default_height       = DEFAULT_HEIGHT,
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
								 width      = default_width,			 height       = default_height,
														red_bits   = default_red_bits,	 green_bits   = default_green_bits,
														blue_bits  = default_blue_bits,  alpha_bits   = default_alpha_bits,
														depth_bits = default_depth_bits, stencil_bits = default_stencil_bits;

							 int swap_interval = default_swap_interval;

							 int clear_buffer_bit = int(default_clear_buffer_bit);

							 int window_flags = int(default_window_flags);
						 };

class game_t {
	struct window_t {
		std::shared_ptr<SDL_Window> ptr;
		glm::vec2 bounds = { 640, 480 };

		operator SDL_Window*() const {
			return ptr.get();
		}
	};

	std::shared_ptr<std::nullptr_t> sdl, ctx;

	auto init_check(const std::string& msg, bool ok) {
		if (ok)
			std::cout << "[INIT] " << msg << " OK!" << std::endl;
		else {
			std::cout << "[INIT] " << msg << " failed!" << std::endl;
			throw std::runtime_error(SDL_GetError());
		}
	}

	protected:
	window_t				 window;
	event_manager		 events;
	keyboard_manager keyboard;
	mouse_manager		 mouse;
	projection_t		 proj;
	camera_t				 view;
	float						 avg_fps, fps, cur_frame_ticks = .0f;
	boost::filesystem::path assets = boost::filesystem::current_path() / "assets";
	bool quit = false;
  
  GLuint plane_VAO;
  program_t plane_shader;
  glm::mat4 plane_model;
  
  program_t info_shader;

public:
	virtual void pre_init() {}
	virtual void init() = 0;
	virtual void update() = 0;
	virtual void render(const float&) = 0;

	template <class C = configurator<>> auto start(const C& c) {
		pre_init();

		init_check("SDL_Init()", SDL_Init(SDL_INIT_VIDEO) >= 0);
		auto sdl = std::shared_ptr<std::nullptr_t>(nullptr, [](std::nullptr_t) {
      SDL_Quit();
    });

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		window.bounds.x = c.width;
		window.bounds.y = c.height;
		proj.set_ratio(window.bounds);
    view.look_at(glm::vec3(0.f, 1.5f, 3.f),
                 glm::vec3(0.f, 1.f, 0.f));

		window.ptr = std::shared_ptr<SDL_Window>(SDL_CreateWindow("twwepdnutis",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					window.bounds.x,
					window.bounds.y,
					SDL_WINDOW_OPENGL bitor SDL_WINDOW_SHOWN bitor c.window_flags),
				SDL_DestroyWindow);
		init_check("SDL_CreateWindow()", bool(window.ptr));

		auto ctx = SDL_GL_CreateContext(window);
		auto ctx_ptr = std::shared_ptr<std::nullptr_t>(nullptr, [ctx](SDL_GLContext _ctx) {
				SDL_GL_DeleteContext(_ctx);
				});
		init_check("SDL_GL_CreateContext()", bool(ctx));

		init_check("glewInit", gladLoadGL());

#ifdef GLAD_DEBUG
		glad_set_pre_callback(pre_gl_call);
#endif
		glad_set_post_callback(post_gl_call);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,		 c.red_bits);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,	 c.green_bits);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,		 c.blue_bits);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,	 c.alpha_bits);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,	 c.depth_bits);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, c.stencil_bits);

		SDL_GL_SetSwapInterval(c.swap_interval);

		glViewport(0, 0, window.bounds.x, window.bounds.y);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		std::cout << "[INFO] Vendor:   " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "[INFO] Renderer: " << glGetString(GL_RENDERER) << std::endl;
		std::cout << "[INFO] Version:  " << glGetString(GL_VERSION) << std::endl;
		std::cout << "[INFO] GLSL:     " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

		events.quit = [&](const SDL_QuitEvent&) {
			quit = true;
		};

		events.window_event = [&](const SDL_WindowEvent& e) {
			if (e.event == SDL_WINDOWEVENT_RESIZED) {
				window.bounds.x = static_cast<float>(e.data1);
				window.bounds.y = static_cast<float>(e.data2);
				proj.set_ratio(window.bounds);

				glViewport(0, 0, e.data1, e.data2);
			}
		};

		keyboard.link_event_manager(events);
		mouse.link_event_manager(events);
    
#if !(defined(NO_GRID) && defined(NO_INFO))
    float plane_vertices[] = {
      1.f,  1.f, 0.0f,  1.0f, 1.0f,
      1.f, -1.f, 0.0f,  1.0f, 0.0f,
      -1.f, -1.f, 0.0f,  0.0f, 0.0f,
      -1.f,  1.f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int plane_indices[] = {
      0, 1, 3,
      1, 2, 3
    };
    
    GLuint VBO, EBO;
    glGenVertexArrays(1, &plane_VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(plane_VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
#endif
    
#ifndef NO_GRID
    plane_shader.load().attach(vertex_f(assets / "grid.vert.glsl"),
                               fragment_f(assets / "grid.frag.glsl"))
    .link()
    .done();
    
    plane_model = glm::scale(glm::rotate(plane_model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)), glm::vec3(10.f, 10.f, 0.f));
#endif
    
#ifndef NO_INFO
    info_shader.load().attach(vertex_f(assets / "font.vert.glsl"),
                              fragment_f(assets / "font.frag.glsl"))
    .link()
    .done();
#endif

		init();

		constexpr auto max_frame_skip = 10;
		constexpr auto frame_rate			= std::chrono::duration<std::chrono::high_resolution_clock::rep, std::ratio<1, 60>>{1};
		constexpr auto frame_rate_c		= std::chrono::duration_cast<std::chrono::duration<float>>(frame_rate).count();
		auto					 next_tick				= std::chrono::high_resolution_clock::now() + frame_rate;

		auto  last_frame_tick = std::chrono::high_resolution_clock::now();
		std::vector<float> fps_avg_vec;
		fps_avg_vec.reserve(10);

		while (not quit) {
			int loops = 0;
			while (std::chrono::high_resolution_clock::now() > next_tick and loops < max_frame_skip) {
				events.poll();

				update();

				mouse.reset();

				next_tick += frame_rate;
				loops++;
			}

			glClear(c.clear_buffer_bit);

			render(std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() + frame_rate - next_tick).count() / frame_rate_c);
      
#ifndef NO_GRID
      plane_shader.use([&]() {
        set_mvp(plane_model, view, proj);
        set_uniform<float>("iResolution", window.bounds.x, window.bounds.y);
        glBindVertexArray(plane_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      });
#endif
      
#ifndef NO_INFO
      info_shader.use([&]() {
        set_uniform<float>("iResolution", window.bounds.x, window.bounds.y);
        set_uniform<float>("FPS", avg_fps);
        glBindVertexArray(plane_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      });
#endif
      
			SDL_GL_SwapWindow(window);

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


#endif /* base_game_h */
