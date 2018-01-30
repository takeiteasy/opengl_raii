//
//  texture.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 19/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef texture_h
#define texture_h

#include "filesystem.hpp"
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
  int w, h, bpp;
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

void load(texture_t& tex, const path& p) {
  if (!tex)
    generate_texture(tex);
  
  bind(tex, [&]() {
    if (not p.exists() or not p.is_file())
      throw std::runtime_error("texture path \"" + p.str() + "\" is not a file or doesn't exist");
    
    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(p.str().c_str(), "r"), &std::fclose);
    if (!fp)
      throw std::runtime_error("failed to load: \"" + p.str() + "\"");
    
    char dummy;
    std::fseek(fp.get(), 4 * sizeof(char) + 4 * sizeof(short int), SEEK_SET);
    std::fread(&tex.w,   sizeof(short), 1, fp.get());
    std::fread(&tex.h,   sizeof(short), 1, fp.get());
    std::fread(&tex.bpp, sizeof(char),  1, fp.get());
    std::fread(&dummy,   sizeof(char),  1, fp.get());
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    auto data = std::shared_ptr<GLubyte>(static_cast<GLubyte*>(std::malloc(tex.w * tex.h * (tex.bpp / 8) * sizeof(GLubyte))), std::free);
    switch(tex.bpp) {
      case 24: {
        for(int i = 0; i < (tex.w * tex.h); i++){
          data.get()[i * 3 + 2] = std::fgetc(fp.get());
          data.get()[i * 3 + 1] = std::fgetc(fp.get());
          data.get()[i * 3 + 0] = std::fgetc(fp.get());
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.w, tex.h, 0, GL_RGB, GL_UNSIGNED_BYTE, data.get());
        break;
      }
      case 32: {
        for (int i = 0; i < (tex.w * tex.h); i++){
          data.get()[i * 4 + 2] = std::fgetc(fp.get());
          data.get()[i * 4 + 1] = std::fgetc(fp.get());
          data.get()[i * 4 + 0] = std::fgetc(fp.get());
          data.get()[i * 4 + 3] = std::fgetc(fp.get());
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
        break;
      }
      default:
        throw std::runtime_error("invalid BPP (" + std::to_string(tex.bpp) + ") for \"" + p.str() + "\"");
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);
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
