//
//  projection.hpp
//  twwfpdnutis
//
//  Created by George Watson on 08/05/2017.
//
//

#ifndef projection_h
#define projection_h

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class projection_t {
	float _fov;
	float _ratio;
	float _near;
	float _far;
	glm::mat4 _proj;
	
	auto update() {
		_proj = glm::perspective(_fov, _ratio, _near, _far);
	}
	
public:
	projection_t(): _fov(45.f), _ratio(1.3333333333333333f), _near(.1f), _far(1000.f) {
		update();
	}
	
	auto set(float fov, float ratio, float near, float far) {
		_fov	 = fov;
		_ratio = ratio;
		_near  = near;
		_far	 = far;
		update();
	}
	
	auto set_fov(float fov) {
		_fov = fov;
		update();
	}
	
	auto set_ratio(float ratio) {
		_ratio = ratio;
		update();
	}
	
	auto set_ratio(float w, float h) {
		_ratio = w / h;
		update();
	}
	
	auto set_ratio(const glm::vec2& v) {
		_ratio = v.x / v.y;
		update();
	}
	
	auto set_near_clip(float near) {
		_near = near;
		update();
	}
	
	auto set_far_clip(float far) {
		_far = far;
		update();
	}
	
	operator glm::mat4() {
		return _proj;
	}
};

#endif /* projection_h */
