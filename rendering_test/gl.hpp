//
//  gl.h
//  rendering_test
//
//  Created by Kirisame Marisa on 16/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef gl_h
#define gl_h

#include "3rdparty/glad.h"

namespace helper {
  namespace gl {
    template<void (*func)(GLuint)> struct ptr_deleter_t {
      typedef GLuint pointer;
      void operator()(GLuint id) { func(id); }
    };
    
    template <typename T> class ptr_t  {
    protected:
      typedef T ptr_type;
      ptr_type ptr;
      
    public:
      ptr_t<T>& operator =(GLuint id) {
        ptr.reset();
        ptr = ptr_type(id);
        return *this;
      }
      
      T& operator ()(GLuint id) {
        ptr.reset();
        ptr = ptr_type(id);
        return ptr;
      }
      
      operator GLuint() {
        return ptr.get();
      }
    };
  }
}

#endif /* gl_h */
