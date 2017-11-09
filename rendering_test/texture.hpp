//
//  texture.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 19/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef texture_h
#define texture_h

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "3rdparty/stb_image.h"

#include "gl.hpp"

namespace helper {
  namespace gl {
    void delete_texture(GLuint id) {
      std::cout << "[LOG] Releasing texture #" << id << std::endl;
      glDeleteTextures(1, &id);
    }
  }
}

typedef std::map<GLenum, GLint> sampler_t;

auto create_sampler(sampler_t& s) {}
template <typename T1=GLenum, typename T2=GLint, typename ... Ts> auto create_sampler(sampler_t& s, const T1& v1, const T2& v2, const Ts& ... vs) {
  s.insert({ v1, v2 });
  create_sampler(s, vs...);
}

struct texture_t: public helper::gl::ptr_t<std::unique_ptr<GLuint, helper::gl::ptr_deleter_t<helper::gl::delete_texture>>> {
  int w, h, c;
};

void generate_texture(texture_t& tex) {
  GLuint id;
  glGenTextures(1, &id);
  tex(id);
}

void bind(texture_t& tex) {
  glBindTexture(GL_TEXTURE_2D, tex);
}

void bind(texture_t& tex, const std::function<void()>& fn) {
  glBindTexture(GL_TEXTURE_2D, tex);
  fn();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void load(texture_t& tex, const boost::filesystem::path& p) {
  if (!tex)
    generate_texture(tex);
  
  bind(tex, [&]() {
    if (not boost::filesystem::exists(p) or not boost::filesystem::is_regular_file(p))
      throw std::runtime_error("texture path \"" + p.string() + "\" is not a file or doesn't exist");
    
    unsigned char* data = stbi_load(p.string().c_str(), &tex.w, &tex.h, &tex.c, 0);
    if (data == nullptr)
      throw std::runtime_error("Failed to load " + p.string());
    
    GLenum format;
    switch (tex.c) {
      case 1:
        format = GL_RED;
        break;
      case 4:
        format = GL_RGBA;
        break;
      default:
        format = GL_RGB;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex.w, tex.h, 0, format, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
  });
}

void add_sampler(texture_t& t, sampler_t& s) {
  glBindTexture(GL_TEXTURE_2D, t);
  for (const auto& kv : s)
    glTexParameteri(GL_TEXTURE_2D, kv.first, kv.second);
  glBindTexture(GL_TEXTURE_2D, 0);
}

auto add_sampleri() {}
auto add_samplerf() {}

template <typename T1=GLenum, typename T2=GLint, typename ... Ts> auto add_sampleri(const T1& v1, const T2& v2, const Ts& ... vs) {
  glTexParameteri(GL_TEXTURE_2D, v1, v2);
  add_sampleri(vs...);
}

template <typename T1=GLenum, typename T2=GLfloat, typename ... Ts> auto add_samplerf(const T1& v1, const T2& v2, const Ts& ... vs) {
  glTexParameterf(GL_TEXTURE_2D, v1, v2);
  add_samplerf(vs...);
}

#endif /* texture_h */
