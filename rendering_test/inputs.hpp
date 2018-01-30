//
//  events.hpp
//  twwfpdnutis
//
//  Created by George Watson on 07/05/2017.
//
//

#ifndef inputs_h
#define inputs_h

#include <functional>
#include <map>

#include <SDL2/SDL_events.h>

/* Stolen from: https://github.com/antonte/sdlpp */
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
