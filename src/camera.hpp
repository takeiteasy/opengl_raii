//
//  camera.hpp
//  twwfpdnutis
//
//  Created by George Watson on 11/05/2017.
//
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

class camera_t {
	glm::vec3 pos   = {0.f, 0.f,  0.f},
		front					= {0.f, 0.f, -1.f},
		up						= {0.f, 1.f,  0.f},
		right,
		world					= {0.f, 1.f, 0.f};
	GLfloat   yaw   = -90.f,
						pitch = 0.f,
						sensitivity = .25f,
						move_speed  = 0.05f,
						zoom  = 45.f;

	auto update() {
		front = glm::normalize(glm::vec3
				cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
				sin(glm::radians(pitch)),
				sin(glm::radians(yaw)) * cos(glm::radians(pitch))));

		right = glm::normalize(glm::cross(front, world));
		up    = glm::normalize(glm::cross(right, front));
	}

	public:
	void look_at(
			glm::vec3 _pos   = { .0f, .0f, .0f },
			glm::vec3 _up    = { .0f, 1.f, .0f},
			GLfloat   _yaw   = -90.f,
			GLfloat   _pitch =  0.f) {
		pos   = _pos;
		up    = _up;
		yaw   = _yaw;
		pitch = _pitch;
		update();
	}

	void move(const VIEW_DIRECTION& d) {
		switch (d) {
			case UP:
				pos += move_speed * up;
				break;
			case DOWN:
				pos -= move_speed * up;
				break;
			case LEFT:
				pos -= glm::normalize(glm::cross(front, up)) * move_speed;
				break;
			case RIGHT:
				pos += glm::normalize(glm::cross(front, up)) * move_speed;
				break;
			case FORWARD:
				pos += move_speed * front;
				break;
			case BACK:
				pos -= move_speed * front;
				break;
		}

		update();
	}

	void look(const glm::vec2& vel) {
		yaw  += vel.x * sensitivity;
		pitch = helper::clamp(pitch + (vel.y * sensitivity), -89.0f, 89.0f);
		update();
	}

	auto set_sensitivity(float _sensitivity) {
		sensitivity = _sensitivity;
	}

	auto set_move_speed(float _move_speed) {
		move_speed = _move_speed;
	}

	auto view() {
		return glm::lookAt(pos, pos + front, up);
	}

	operator glm::mat4() {
		return view();
	}
};

#endif /* camera_h */
