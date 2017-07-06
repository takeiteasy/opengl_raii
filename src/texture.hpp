//
//  texture.hpp
//  twwfpdnutis
//
//  Created by George Watson on 09/05/2017.
//
//

#ifndef texture_h
#define texture_h

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"

#include "gl_object.hpp"

namespace helper {
	void gl_delete_texture(GLuint id) {
		std::cout << "[LOG] Releasing texture #" << id << std::endl;
		glDeleteTextures(1, &id);
	}
}

class texture_t: public helper::gl::obj_t<std::unique_ptr<GLuint, helper::gl::obj_deleter_t<helper::gl_delete_texture>>> {
	int w, h, c;

public:
	static auto done() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	auto& create() {
		GLuint id;
		glGenTextures(1, &id);
		ptr = ptr_type(id);
		std::cout << "[LOG] Creating texture #" << *this << std::endl;
		return *this;
	}

	auto& bind() {
		glBindTexture(GL_TEXTURE_2D, *this);
		return *this;
	}

	auto bind(const std::function<void()>& f) {
		glBindTexture(GL_TEXTURE_2D, *this);
		f();
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	auto& load(const boost::filesystem::path& p) {
		bind();

		if (not boost::filesystem::exists(p) or not boost::filesystem::is_regular_file(p))
			throw std::runtime_error("texture path \"" + p.string() + "\" is not a file or doesn't exist");

		unsigned char* data = stbi_load(p.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
		if (data == nullptr)
			throw std::runtime_error("Failed to load " + p.string());

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);

		return *this;
	}

	auto& create(const boost::filesystem::path& p) {
		create();
		return load(p);
	}

	auto& tex_params_i() {
		return *this;
	}

	auto& tex_params_f() {
		return *this;
	}

	template <typename T1=GLenum, typename T2=GLint, typename ... Ts> auto& tex_params_i(const T1& v1, const T2& v2, const Ts& ... vs) {
		bind();
		glTexParameteri(GL_TEXTURE_2D, v1, v2);
		tex_params_i(vs...);
		return *this;
	}

	template <typename T1=GLenum, typename T2=GLfloat, typename ... Ts> auto& tex_params_f(const T1& v1, const T2& v2, const Ts& ... vs) {
		bind();
		glTexParameterf(GL_TEXTURE_2D, v1, v2);
		tex_params_f(vs...);
		return *this;
	}

	template<typename T> auto& tex_params(GLenum, T);

	auto& mipmap() {
		bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		return *this;
	}

	auto width() const {
		return w;
	}

	auto height() const {
		return h;
	}
};

template<> auto& texture_t::tex_params<float>(GLenum t, float v) {
	bind();
	glTexParameterf(GL_TEXTURE_2D, t, v);
	return *this;
}

template<> auto& texture_t::tex_params<int>(GLenum t, int v) {
	bind();
	glTexParameteri(GL_TEXTURE_2D, t, v);
	return *this;
}

auto tex_params_i() {}
auto tex_params_f() {}

template <typename T1=GLenum, typename T2=GLint, typename ... Ts> auto tex_params_i(const T1& v1, const T2& v2, const Ts& ... vs) {
	glTexParameteri(GL_TEXTURE_2D, v1, v2);
	tex_params_i(vs...);
}

template <typename T1=GLenum, typename T2=GLfloat, typename ... Ts> auto tex_params_f(const T1& v1, const T2& v2, const Ts& ... vs) {
	glTexParameterf(GL_TEXTURE_2D, v1, v2);
	tex_params_f(vs...);
}

template<typename T> auto tex_params(GLenum, T);

template<> auto tex_params<float>(GLenum t, float v) {
	glTexParameterf(GL_TEXTURE_2D, t, v);
}

template<> auto tex_params<int>(GLenum t, int v) {
	glTexParameteri(GL_TEXTURE_2D, t, v);
}

#endif /* texture_h */
