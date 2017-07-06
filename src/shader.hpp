//
//  shader.hpp
//  twwfpdnutis
//
//  Created by George Watson on 08/05/2017.
//
//

#ifndef shader_h
#define shader_h

#include <vector>
#include <fstream>
#include <streambuf>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "gl_object.hpp"

#define GLSL(VERSION, CODE) "#version " #VERSION "\n" #CODE
#define GLSL330(CODE) GLSL(330, CODE)

namespace helper {
	void gl_delete_program(GLuint id) {
		std::cout << "[LOG] Releasing shader #" << id << std::endl;
		glDeleteProgram(id);
	}
}

class program_t: public helper::gl::obj_t<std::unique_ptr<GLuint, helper::gl::obj_deleter_t<helper::gl_delete_program>>> {
	auto check() {
		GLint success;
		glGetProgramiv(*this, GL_LINK_STATUS, &success);
		if (success == GL_FALSE) {
			GLint length = 0;
			glGetProgramiv(*this, GL_INFO_LOG_LENGTH, &length);
			
			if (length) {
				std::vector<GLchar> log(static_cast<unsigned int>(length));
				glGetProgramInfoLog(*this, length, nullptr, log.data());
				
				throw std::runtime_error(std::string(log.begin(), log.end()));
			}
		}
	}
	
public:
	auto& load() {
		ptr = ptr_type(glCreateProgram());
		std::cout << "[LOG] Creating shader #" << *this << std::endl;
		return *this;
	}
	
	static auto done() {
		glUseProgram(0);
	}
	
	template<typename A> auto& attach(A head) {
		glAttachShader(*this, head);
		glDeleteShader(head);
		return *this;
	}
	
	template<typename A, typename...B> auto& attach(A head, B...tail) {
		attach(head);
		attach(tail...);
		return *this;
	}
	
	template<typename...A> auto& load(A...args) {
		load();
		attach(args...);
		return *this;
	}
	
	auto& link() {
		glLinkProgram(*this);
		check();
		return *this;
	}
	
	auto get_uniform_loc(const std::string& s) {
		return glGetUniformLocation(*this, s.c_str());
	}
	
	auto use() {
		glUseProgram(*this);
	}
	
	auto use(const std::function<void()>& f) {
		glUseProgram(*this);
		f();
		glUseProgram(0);
	}
	
	template<typename T> auto& set_uniform(const std::string&, T);
	template<typename T> auto& set_uniform(const std::string&, T, T);
	template<typename T> auto& set_uniform(const std::string&, T, T, T);
	
	auto& set_uniform_vec3(const std::string& s, const glm::vec3& v) {
		use();
		glUniform3f(get_uniform_loc(s), v.x, v.y, v.z);
		return *this;
	}
	
	auto& set_uniform_mat4(const std::string& s, const glm::mat4& m) {
		use();
		glUniformMatrix4fv(get_uniform_loc(s), 1, GL_FALSE, glm::value_ptr(m));
		return *this;
	}
	
	auto& set_vp(const glm::mat4& v, const glm::mat4& p) {
		use();
		glUniformMatrix4fv(get_uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(get_uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(p));
		return *this;
	}
	
	auto& set_mvp(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p) {
		use();
		glUniformMatrix4fv(get_uniform_loc("model"), 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(get_uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(get_uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(p));
		return *this;
	}
};

template<> auto& program_t::set_uniform<int>(const std::string& s, int a) {
	use();
	glUniform1i(get_uniform_loc(s), a);
	return *this;
}

template<> auto& program_t::set_uniform<bool>(const std::string& s, bool a) {
	use();
	glUniform1i(get_uniform_loc(s), a);
	return *this;
}

template<> auto& program_t::set_uniform<float>(const std::string& s, float a) {
	use();
	glUniform1f(get_uniform_loc(s), a);
	return *this;
}

template<> auto& program_t::set_uniform<float>(const std::string& s, float a, float b) {
	use();
	glUniform2f(get_uniform_loc(s), a, b);
	return *this;
}

template<> auto& program_t::set_uniform<float>(const std::string& s, float a, float b, float c) {
	use();
	glUniform3f(get_uniform_loc(s), a, b, c);
	return *this;
}

GLint get_current_shader_id() {
	GLint id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);
	return id;
}

template<typename T> auto set_uniform(const std::string&, T);
template<typename T> auto set_uniform(const std::string&, T, T);
template<typename T> auto set_uniform(const std::string&, T, T, T);

template<> auto set_uniform<int>(const std::string& s, int a) {
	glUniform1i(glGetUniformLocation(get_current_shader_id(), s.c_str()), a);
}

template<> auto set_uniform<bool>(const std::string& s, bool a) {
	glUniform1i(glGetUniformLocation(get_current_shader_id(), s.c_str()), a);
}

template<> auto set_uniform<float>(const std::string& s, float a) {
	glUniform1f(glGetUniformLocation(get_current_shader_id(), s.c_str()), a);
}

template<> auto set_uniform<float>(const std::string& s, float a, float b) {
	glUniform2f(glGetUniformLocation(get_current_shader_id(), s.c_str()), a, b);
}

template<> auto set_uniform<float>(const std::string& s, float a, float b, float c) {
	glUniform3f(glGetUniformLocation(get_current_shader_id(), s.c_str()), a, b, c);
}

auto set_uniform_vec3(const std::string& s, const glm::vec3& v) {
	glUniform3f(get_current_shader_id(), v.x, v.y, v.z);
}

auto set_uniform_mat4(const std::string& s, const glm::mat4& m) {
	glUniformMatrix4fv(glGetUniformLocation(get_current_shader_id(), s.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

auto set_vp(const glm::mat4& v, const glm::mat4& p) {
	GLint id = get_current_shader_id();
	glUniformMatrix4fv(glGetUniformLocation(id, "view"), 1, GL_FALSE, glm::value_ptr(v));
	glUniformMatrix4fv(glGetUniformLocation(id, "projection"), 1, GL_FALSE, glm::value_ptr(p));
}

auto set_mvp(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p) {
	GLint id = get_current_shader_id();
	glUniformMatrix4fv(glGetUniformLocation(id, "model"), 1, GL_FALSE, glm::value_ptr(m));
	glUniformMatrix4fv(glGetUniformLocation(id, "view"), 1, GL_FALSE, glm::value_ptr(v));
	glUniformMatrix4fv(glGetUniformLocation(id, "projection"), 1, GL_FALSE, glm::value_ptr(p));
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

auto shader_path(GLenum t, const boost::filesystem::path& p) {
	if (not boost::filesystem::exists(p) or not boost::filesystem::is_regular_file(p))
		throw std::runtime_error("shader path \"" + p.string() + "\" is not a file or doesn't exist");
	
	std::ifstream ifs(p.string());
	if (not ifs.is_open())
		throw std::runtime_error("Failed to open \"" + p.string() + "\"");
	
	std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	return shader(t, str.c_str());
}

auto vertex			= std::bind(shader,			 GL_VERTEX_SHADER,   std::placeholders::_1);
auto fragment	  = std::bind(shader,			 GL_FRAGMENT_SHADER, std::placeholders::_1);
auto geometry	  = std::bind(shader,			 GL_GEOMETRY_SHADER, std::placeholders::_1);
auto vertex_f		= std::bind(shader_path, GL_VERTEX_SHADER,	 std::placeholders::_1);
auto fragment_f = std::bind(shader_path, GL_FRAGMENT_SHADER, std::placeholders::_1);
auto geometry_f = std::bind(shader_path, GL_GEOMETRY_SHADER, std::placeholders::_1);

#endif /* shader_h */
