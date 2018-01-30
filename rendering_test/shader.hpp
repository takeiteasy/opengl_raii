//
//  shader.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 16/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef shader_h
#define shader_h

#include <vector>
#include <fstream>
#include <streambuf>
#include <map>
#include <regex>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "helpers.hpp"
#include "gl.hpp"

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

namespace helper {
  namespace gl {
    void delete_program(GLuint id) {
      std::cout << "[LOG] Releasing shader #" << id << std::endl;
      glDeleteProgram(id);
    }
    
    auto shader(GLenum t, const char* c) {
      auto r = glCreateShader(t);
      glShaderSource(r, 1, &c, nullptr);
      glCompileShader(r);
      
      GLint success;
      glGetShaderiv(r, GL_COMPILE_STATUS, &success);
      if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(r, GL_INFO_LOG_LENGTH, &length);
        
        if (length) {
          std::vector<GLchar> log(static_cast<unsigned int>(length));
          glGetShaderInfoLog(r, length, nullptr, log.data());
          
          throw std::runtime_error(std::string(log.begin(), log.end()));
        }
      }
      
      return r;
    }
  }
}

static auto vertex   = std::bind(helper::gl::shader, GL_VERTEX_SHADER,   std::placeholders::_1);
static auto fragment = std::bind(helper::gl::shader, GL_FRAGMENT_SHADER, std::placeholders::_1);
static auto geometry = std::bind(helper::gl::shader, GL_GEOMETRY_SHADER, std::placeholders::_1);

struct shader_t: public helper::gl::ptr_t<std::unique_ptr<GLuint, helper::gl::ptr_deleter_t<helper::gl::delete_program>>> {
  std::map<std::string, GLuint> locations;
};

template <typename T=std::string, typename ... Ts> auto add_locations(shader_t& s, const Ts& ... locs) {
  for (const auto& loc: { locs... })
    s.locations.insert({loc, glGetUniformLocation(s, loc.c_str())});
}

GLuint get_location(shader_t& s, const std::string& k) {
  return (s.locations.find(k) != s.locations.end() ? s.locations[k] : -1);
}

auto load(shader_t& s, const char* vert, const char* frag, const char* geom = nullptr) {
  s(glCreateProgram());
  
  glAttachShader(s, vertex(vert));
  if (geom)
    glAttachShader(s, geometry(geom));
  glAttachShader(s, fragment(frag));
  glLinkProgram(s);
  
  GLint success;
  glGetProgramiv(s, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLint length = 0;
    glGetProgramiv(s, GL_INFO_LOG_LENGTH, &length);
    
    if (length) {
      std::vector<GLchar> log(static_cast<unsigned int>(length));
      glGetProgramInfoLog(s, length, nullptr, log.data());
      
      throw std::runtime_error(std::string(log.begin(), log.end()));
    }
  }
  
  const auto post_process = [&s](std::string str) {
    static const std::regex re_first(R"(uniform \S+ (.*);)", std::regex_constants::icase);
    static const std::regex re_second(R"((\S+)\[.*\])", std::regex_constants::icase);
    static const std::regex re_third(R"(\,\s*)", std::regex_constants::icase);
    std::smatch matches;
    while(std::regex_search(str, matches, re_first)) {
      std::string m = matches[1];
      if (m.find(',') != std::string::npos) {
        std::vector<std::string> tokens {
          std::sregex_token_iterator(m.begin(), m.end(), re_third, -1),
          std::sregex_token_iterator()
        };
        
        for (auto& token: tokens) {
          if (m.find('[') and m.find(']')) {
            std::smatch token_matches;
            while(std::regex_search(token, token_matches, re_second)) {
              std::string m = token_matches[1];
              s.locations.insert({m, glGetUniformLocation(s, m.c_str())});
              token = token_matches.suffix();
            }
          } else
            s.locations.insert({token, glGetUniformLocation(s, token.c_str())});
        }
      } else
        s.locations.insert({m, glGetUniformLocation(s, m.c_str())});
      str = matches.suffix();
    }
  };
  
  post_process(std::string(vert));
  if (geom)
    post_process(std::string(geom));
  post_process(std::string(frag));
}

auto load(shader_t& s, const path& vert, const path& frag, const path& geom = "") {
  std::string v = __load_file_to_mem(vert);
  std::string f = __load_file_to_mem(frag);
  std::string g = "";
  if (!geom.str().empty())
    g = __load_file_to_mem(geom);
  
  load(s, v.c_str(), f.c_str(), (geom.str().empty() ? nullptr : g.c_str()));
}

static shader_t* __shader__ = nullptr;

template<typename T> auto set_uniform(const std::string&, T);
template<typename T> auto set_uniform(const std::string&, T, T);
template<typename T> auto set_uniform(const std::string&, T, T, T);

template<> auto set_uniform<int>(const std::string& s, int a) {
  glUniform1i(get_location(*__shader__, s), a);
}

template<> auto set_uniform<bool>(const std::string& s, bool a) {
  glUniform1i(get_location(*__shader__, s), a);
}

template<> auto set_uniform<float>(const std::string& s, float a) {
  glUniform1f(get_location(*__shader__, s), a);
}

template<> auto set_uniform<glm::vec2>(const std::string& s, glm::vec2 a) {
  glUniform2f(get_location(*__shader__, s), a.x, a.y);
}

template<> auto set_uniform<glm::vec3>(const std::string& s, glm::vec3 a) {
  glUniform3f(get_location(*__shader__, s), a.x, a.y, a.z);
}

template<> auto set_uniform<glm::vec4>(const std::string& s, glm::vec4 a) {
  glUniform4f(get_location(*__shader__, s), a.x, a.y, a.z, a.w);
}

template<> auto set_uniform<glm::mat3>(const std::string& s, glm::mat3 a) {
  glUniformMatrix3fv(get_location(*__shader__, s), 1, GL_FALSE, &a[0][0]);
}

template<> auto set_uniform<glm::mat4>(const std::string& s, glm::mat4 a) {
  glUniformMatrix4fv(get_location(*__shader__, s), 1, GL_FALSE, &a[0][0]);
}

template<> auto set_uniform<float>(const std::string& s, float a, float b) {
  glUniform2f(get_location(*__shader__, s), a, b);
}

template<> auto set_uniform<float>(const std::string& s, float a, float b, float c) {
  glUniform3f(get_location(*__shader__, s), a, b, c);
}

auto use(shader_t& s, const std::function<void()>& fn) {
  glUseProgram(s);
  __shader__ = &s;
  fn();
  __shader__ = nullptr;
  glUseProgram(0);
}

#endif /* shader_h */
