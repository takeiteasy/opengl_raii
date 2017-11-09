//
//  helpers.hpp
//  rendering_test
//
//  Created by Kirisame Marisa on 17/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#ifndef helpers_h
#define helpers_h

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

std::string __load_file_to_mem(const boost::filesystem::path& p) {
  if (not boost::filesystem::exists(p) or not boost::filesystem::is_regular_file(p))
    throw std::runtime_error("shader path \"" + p.string() + "\" is not a file or doesn't exist");
  
  std::ifstream ifs(p.string());
  if (not ifs.is_open())
    throw std::runtime_error("Failed to open \"" + p.string() + "\"");
  
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

#endif /* helpers_h */
