//
//  gl_object.hpp
//  twwfpdnutis
//
//  Created by George Watson on 08/05/2017.
//
//

#ifndef gl_object_h
#define gl_object_h

#include <memory>

#include "3rdparty/glad.h"

namespace helper {
	namespace gl {
		template<void (*func)(GLuint)> struct obj_deleter_t {
			typedef GLuint pointer;
			void operator()(GLuint id) { func(id); }
		};
		
		template <typename T> class obj_t  {
		protected:
			typedef T ptr_type;
			ptr_type ptr;
			
		public:
			obj_t<T>& operator =(GLuint id) {
				ptr.reset();
				ptr = ptr_type(id);
				return *this;
			}
			
			operator GLuint() {
				return ptr.get();
			}
		};
	}
}

#endif /* gl_object_h */
