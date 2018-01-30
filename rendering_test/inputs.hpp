/*
 This source file is part of sdlpp (C++ wrapper for SDL2)
 Copyright (c) 2017 Anton Te
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#ifndef inputs_h
#define inputs_h

#include <functional>
#include <map>

#include <SDL2/SDL_events.h>

class event_manager {
#define SDL_EVENTS \
  EVENT(SDL_AUDIODEVICEADDED, audio_device_added, adevice); \
  EVENT(SDL_AUDIODEVICEREMOVED, audio_device_removed, adevice); \
  EVENT(SDL_CONTROLLERAXISMOTION, controller_axis_motion, caxis); \
  EVENT(SDL_CONTROLLERBUTTONDOWN, controller_button_down, cbutton); \
  EVENT(SDL_CONTROLLERBUTTONUP, controller_button_up, cbutton); \
  EVENT(SDL_CONTROLLERDEVICEADDED, controller_device_added, cdevice); \
  EVENT(SDL_CONTROLLERDEVICEREMOVED, controller_device_removed, cdevice); \
  EVENT(SDL_CONTROLLERDEVICEREMAPPED, controller_device_remapped, cdevice); \
  EVENT(SDL_DOLLARGESTURE, dollar_gesture, dgesture); \
  EVENT(SDL_DOLLARRECORD, dollar_record, dgesture); \
  EVENT(SDL_DROPFILE, drop_file, drop); \
  EVENT(SDL_FINGERMOTION, finger_motion, tfinger); \
  EVENT(SDL_FINGERDOWN, finger_down, tfinger); \
  EVENT(SDL_FINGERUP, finger_up, tfinger); \
  EVENT(SDL_KEYDOWN, key_down, key); \
  EVENT(SDL_KEYUP, key_up, key); \
  EVENT(SDL_JOYAXISMOTION, joy_axis_motion, jaxis); \
  EVENT(SDL_JOYBALLMOTION, joy_ball_motion, jball); \
  EVENT(SDL_JOYHATMOTION, joy_hat_motion, jhat); \
  EVENT(SDL_JOYBUTTONDOWN, joy_button_down, jbutton); \
  EVENT(SDL_JOYBUTTONUP, joy_button_up, jbutton); \
  EVENT(SDL_JOYDEVICEADDED, joy_device_added, jdevice); \
  EVENT(SDL_JOYDEVICEREMOVED, joy_device_removed, jdevice); \
  EVENT(SDL_MOUSEMOTION, mouse_motion, motion); \
  EVENT(SDL_MOUSEBUTTONDOWN, mouse_button_down, button); \
  EVENT(SDL_MOUSEBUTTONUP, mouse_button_up, button); \
  EVENT(SDL_MOUSEWHEEL, mouse_wheel, wheel); \
  EVENT(SDL_MULTIGESTURE, multi_gesture, mgesture); \
  EVENT(SDL_QUIT, quit, quit); \
  EVENT(SDL_SYSWMEVENT, sys_wm_event, syswm); \
  EVENT(SDL_TEXTEDITING, text_editing, edit); \
  EVENT(SDL_TEXTINPUT, text_input, text); \
  EVENT(SDL_USEREVENT, user_event, user); \
  EVENT(SDL_WINDOWEVENT, window_event, window);
  
public:
#define EVENT(x, y, z) std::function<void(const decltype(SDL_Event::z) &)> y
		SDL_EVENTS
#undef EVENT
  
  auto poll() {
    static SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
#define EVENT(x, y, z) \
        case x: \
          if (y) \
            y(e.z); \
          break;
      SDL_EVENTS
#undef EVENT
        default:
          break;
      }
    }
  }
};

#endif /* inputs_h */
