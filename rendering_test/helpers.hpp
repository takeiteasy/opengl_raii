//
//  helpers.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 17/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef helpers_h
#define helpers_h

#include "3rdparty/filesystem.hpp"

std::string load_file_to_mem(const path& p) {
  if (not p.exists() or not p.is_file())
    throw std::runtime_error("path \"" + p.str() + "\" is not a file or doesn't exist");
  
  std::ifstream ifs(p.str());
  if (not ifs.is_open())
    throw std::runtime_error("Failed to open \"" + p.str() + "\"");
  
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

#endif /* helpers_h */
