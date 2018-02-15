//
//  main.cpp
//  rendering_test
//
//  Created by Kirisame Marisa on 16/07/2017.
//  Copyright © 2017 Kirisame Marisa. All rights reserved.
//

#include <iostream>
#include "base.hpp"

enum { X, Y, Z, W };

enum buffer_index {
  VERTEX_BUFFER,
  COLOUR_BUFFER,
  TEXTURE_BUFFER,
  NORMAL_BUFFER,
  TANGENT_BUFFER,
  INDEX_BUFFER,
  MAX_BUFFER
};

struct vertex_t {
  glm::vec3 position, normal;
  glm::vec2 uv;
  glm::vec4 weights, indices;
  int start, count;
};

struct md5_joint_t {
  char name[64];
  int parent;
  glm::vec3 position;
  glm::quat orientation;
};

struct md5_vertex_t {
  glm::vec2 st;
  int start;
  int count;
};

struct md5_triangle_t {
  int index[3];
};

struct md5_weight_t {
  int joint;
  float bias;
  glm::vec3 position;
};

struct md5_bbox_t {
  glm::vec3 min;
  glm::vec3 max;
};

struct md5_mesh_data_t {
  std::string shader;
  std::vector<md5_vertex_t> vertices;
  std::vector<md5_triangle_t> triangles;
  std::vector<md5_weight_t> weights;
};

struct md5_mesh_t {
  md5_mesh_data_t data;
  texture_t texture;
  std::vector<unsigned int> indices;
  std::vector<glm::vec3> vertices, normals;
  std::vector<glm::vec2> st, weights;
  vertex_array_t vao;
  std::vector<buffer_t> vbo;
};

struct joint_info_t {
  char name[64];
  int parent;
  int flags;
  int start;
};

struct baseframe_joint_t {
  glm::vec3 position;
  glm::quat orientation;
};

struct anim_info_t {
  int curr_frame;
  int next_frame;
  
  double last_time;
  double max_time;
};

struct md5_anim_t {
  int frame_rate;
  std::vector<std::vector<md5_joint_t>> frames;
  std::vector<md5_bbox_t> bboxes;
  anim_info_t anim_info;
};

struct md5_t {
  std::vector<md5_joint_t> base_skeleton, skeleton;
  std::vector<md5_mesh_t> meshes;
  std::map<std::string, md5_anim_t> animations;
};

auto compute_quat_w(glm::quat& q) {
  float t = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
  q.w = (t < 0.0f ? 0.0f : -sqrt(t));
}

//auto md5_prepare_mesh(md5_mesh_t& mesh, const std::vector<md5_joint_t>& skeleton) {
//  std::vector<vertex_t> tmp_verts;
//  tmp_verts.resize(mesh.vertices.size());
//  for (int i = 0; i < mesh.vertices.size(); ++i) {
//    vertex_t& vert = tmp_verts[i];
//    md5_vertex_t& old_vert = mesh.vertices[i];
//    int count = old_vert.count;
//    for (int j = 0; j < count; ++j) {
//      if (j >= 4)
//        break;
//
//      const md5_weight_t& weight = mesh.weights[old_vert.start + j];
//      const md5_joint_t&  joint  = skeleton[weight.joint];
//
//      vert.position  += (joint.position + joint.orientation * weight.position) * weight.bias;
//      vert.indices[j] = static_cast<float>(weight.joint);
//      vert.weights[j] = weight.bias;
//    }
//    vert.uv = old_vert.st;
//  }
//
//  for (int i = 0; i < mesh.triangles.size(); ++i) {
//    glm::vec3& v = tmp_verts[mesh.triangles[i].index[X]].position;
//    glm::vec3 norm = glm::cross(tmp_verts[mesh.triangles[i].index[Z]].position - v,
//                                tmp_verts[mesh.triangles[i].index[Y]].position - v);
//    tmp_verts[mesh.triangles[i].index[X]].normal += norm;
//    tmp_verts[mesh.triangles[i].index[Y]].normal += norm;
//    tmp_verts[mesh.triangles[i].index[Z]].normal += norm;
//  }
//
//  for (auto& v: tmp_verts)
//    v.normal = glm::normalize(v.normal);
//
//  generate_vertex_array(mesh.VAO);
//  generate_array_buffer(mesh.VBO);
//  generate_array_buffer(mesh.EBO);
//
//  bind(mesh.VAO, [&]() {
//    bind_data<vertex_t, GL_ARRAY_BUFFER>(mesh.VBO, tmp_verts);
//    bind_data<md5_triangle_t, GL_ELEMENT_ARRAY_BUFFER>(mesh.EBO, mesh.triangles);
//
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
//    glEnableVertexAttribArray(2);
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, uv));
//    glEnableVertexAttribArray(3);
//    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, weights));
//    glEnableVertexAttribArray(4);
//    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, indices));
//  });
//}
//
//auto md5_interpolate_skeletons(const std::vector<md5_joint_t>& a, const std::vector<md5_joint_t>& b, std::vector<md5_joint_t>& out, float interp) {
//  for (int i = 0; i < a.size(); ++i) {
//    out[i].parent = a[i].parent;
//    out[i].position = a[i].position + interp * (b[i].position - b[i].position);
//    out[i].orientation = glm::slerp(a[i].orientation, b[i].orientation, interp);
//  }
//}

auto md5_skin_mesh(md5_mesh_t& mesh, const std::vector<md5_joint_t>& skeleton) {
  for (int i = 0; i < mesh.vertices.size(); ++i) {
    mesh.st[i] = mesh.data.vertices[i].st;
    mesh.vertices[i] = glm::vec3(0.f);
    for (int j = 0; j < mesh.data.vertices[i].count; ++j) {
      if (j >= 4)
        break;
      
      const md5_weight_t& weight = mesh.data.weights[mesh.data.vertices[i].start + j];
      const md5_joint_t& joint = skeleton[weight.joint];
      mesh.vertices[i] += (joint.position + joint.orientation * weight.position) * weight.bias;
    }
  }
  
  for (int i = 0; i < mesh.vertices.size(); ++i) {
    glm::vec3& v = mesh.vertices[i];
    glm::vec3 normal = glm::normalize(glm::cross(mesh.vertices[i + 1] - v, mesh.vertices[i + 2] - v));
    for (int j = 0; j < 3; ++j)
      mesh.normals[i + j] = normal;
  }
  
  bind_sub_data<glm::vec3, GL_ARRAY_BUFFER>(mesh.vbo[VERTEX_BUFFER], mesh.vertices);
  bind_sub_data<glm::vec2, GL_ARRAY_BUFFER>(mesh.vbo[TEXTURE_BUFFER], mesh.st);
  bind_sub_data<glm::vec3, GL_ARRAY_BUFFER>(mesh.vbo[NORMAL_BUFFER], mesh.normals);
  bind_sub_data<unsigned int, GL_ELEMENT_ARRAY_BUFFER>(mesh.vbo[INDEX_BUFFER], mesh.indices);
}

auto load_anim(std::map<std::string, md5_anim_t>& m) {}
template<typename T=path, typename... Ts> auto load_anim(std::map<std::string, md5_anim_t>& m, const T& p, const Ts& ... ts) {
  if (not p.exists() or not p.is_file())
    throw std::runtime_error("md5anim path \"" + p.str() + "\" is not a file or doesn't exist");
  
  std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(p.str().c_str(), "r"), &std::fclose);
  if (!fp)
    throw std::runtime_error("failed to load: \"" + p.str() + "\"");
  
  std::string fname = p.filename();
  md5_anim_t* anim = &m[fname.substr(0, fname.find_last_of("."))];
  
  char buff[512];
  int version, num_frames, num_joints, num_anim_comps, frame_index;
  std::vector<joint_info_t> joint_infos;
  std::vector<baseframe_joint_t> base_frame;
  std::vector<float> anim_frame_data;
  const auto get_line = std::bind(std::fgets, buff, sizeof(buff), fp.get());
  
  while (!std::feof(fp.get())) {
    get_line();
    
    if (std::sscanf(buff, " MD5Version %d", &version) == 1) {
      if (version != 10)
        throw std::runtime_error("ERROR: Bad animation version");
    }
    else if (std::sscanf(buff, " numFrames %d", &num_frames) == 1) {
      if (num_frames > 0) {
        anim->frames.resize(num_frames);
        anim->bboxes.resize(num_frames);
      }
    }
    else if (std::sscanf(buff, " numJoints %d", &num_joints) == 1) {
      if (num_joints > 0) {
        for (int i = 0; i < num_frames; ++i)
          anim->frames[i].resize(num_joints);
        
        joint_infos.resize(num_joints);
        base_frame.resize(num_joints);
      }
    }
    else if (std::sscanf(buff, " frameRate %d", &anim->frame_rate) == 1) {}
    else if (std::sscanf(buff, " numAnimatedComponents %d", &num_anim_comps) == 1) {
      if (num_anim_comps > 0)
        anim_frame_data.resize(num_anim_comps);
    }
    else if (strncmp(buff, "hierarchy {", 11) == 0) {
      for (int i = 0; i < num_joints; ++i) {
        get_line();
        
        std::sscanf(buff, " %s %d %d %d",
                    joint_infos[i].name,   &joint_infos[i].parent,
                    &joint_infos[i].flags, &joint_infos[i].start);
      }
    }
    else if (std::strncmp(buff, "bounds {", 8) == 0) {
      for (int i = 0; i < num_frames; ++i) {
        get_line();
        
        std::sscanf(buff, " ( %f %f %f ) ( %f %f %f )",
                    &anim->bboxes[i].min.x, &anim->bboxes[i].min.y,
                    &anim->bboxes[i].min.z, &anim->bboxes[i].max.x,
                    &anim->bboxes[i].max.y, &anim->bboxes[i].max.z);
      }
    }
    else if (std::strncmp(buff, "baseframe {", 10) == 0) {
      for (int i = 0; i < num_joints; ++i) {
        get_line();
        
        if (std::sscanf(buff, " ( %f %f %f ) ( %f %f %f )",
                        &base_frame[i].position.x,    &base_frame[i].position.y,    &base_frame[i].position.z,
                        &base_frame[i].orientation.x, &base_frame[i].orientation.y, &base_frame[i].orientation.z) == 6)
          compute_quat_w(base_frame[i].orientation);
      }
    }
    else if (std::sscanf(buff, " frame %d", &frame_index) == 1) {
      for (int i = 0; i < num_anim_comps; ++i)
        std::fscanf(fp.get(), "%f", &anim_frame_data[i]);
      
      for (int i = 0; i < num_joints; ++i) {
        const baseframe_joint_t& base_joint = base_frame[i];
        
        int j = 0;
        glm::vec3 posistion   = base_joint.position;
        glm::quat orientation = base_joint.orientation;
        if (joint_infos[i].flags & 1) /* Tx */ {
          posistion.x = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        if (joint_infos[i].flags & 2) /* Ty */ {
          posistion.y = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        if (joint_infos[i].flags & 4) /* Tz */ {
          posistion.z = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        if (joint_infos[i].flags & 8) /* Qx */ {
          orientation.x = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        if (joint_infos[i].flags & 16) /* Qy */ {
          orientation.y = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        if (joint_infos[i].flags & 32) /* Qz */ {
          orientation.z = anim_frame_data[joint_infos[i].start + j];
          ++j;
        }
        
        compute_quat_w(orientation);
        
        struct md5_joint_t& this_joint = anim->frames[frame_index][i];
        this_joint.parent = joint_infos[i].parent;
        std::strcpy(this_joint.name, joint_infos[i].name);
        
        if (this_joint.parent < 0) {
          this_joint.position = posistion;
          this_joint.orientation = orientation;
        } else {
          struct md5_joint_t& parent_joint = anim->frames[frame_index][joint_infos[i].parent];
          glm::quat tmp = (parent_joint.orientation * posistion) * glm::normalize(glm::inverse(parent_joint.orientation));
          this_joint.position = glm::vec3(tmp.x, tmp.y, tmp.z) + parent_joint.position;
          this_joint.orientation = glm::normalize(parent_joint.orientation * orientation);
        }
      }
    }
  }
  
  anim->anim_info.curr_frame = 0;
  anim->anim_info.next_frame = 1;
  anim->anim_info.last_time = 0;
  anim->anim_info.max_time = 1.0 / anim->frame_rate;
  
  load_anim(m, ts...);
}

template<typename... T, typename=path> auto load(md5_t& m, const path& p, const T& ... ts) {
  if (not p.exists() or not p.is_file())
    throw std::runtime_error("md5mesh path \"" + p.str() + "\" is not a file or doesn't exist");
  
  std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(p.str().c_str(), "r"), &std::fclose);
  if (!fp)
    throw std::runtime_error(p.str());
  
  char buff[512];
  int version, curr_mesh = 0, index_start = 0, num_joints, num_meshes;
  const auto get_line = std::bind(std::fgets, buff, sizeof(buff), fp.get());
  
  while (!std::feof(fp.get())) {
    get_line();
    
    if (std::sscanf(buff, " MD5Version %d", &version) == 1) {
      if (version != 10)
        throw std::runtime_error("ERROR: Bad model version");
    }
    else if (std::sscanf(buff, " numJoints %d", &num_joints) == 1) {
      if (num_joints > 0) {
        m.base_skeleton.resize(num_joints);
        m.skeleton.resize(num_joints);
      }
    }
    else if (std::sscanf(buff, " numMeshes %d", &num_meshes) == 1) {
      if (num_meshes > 0)
        m.meshes.resize(num_meshes);
    }
    else if (std::strncmp(buff, "joints {", 8) == 0) {
      for (int i = 0; i < num_joints; ++i) {
        md5_joint_t& joint = m.base_skeleton[i];
        
        get_line();
        if (std::sscanf(buff, "%s %d ( %f %f %f ) ( %f %f %f )",
                         joint.name, &joint.parent,
                         &joint.position.x, &joint.position.y, &joint.position.z,
                         &joint.orientation.x, &joint.orientation.y, &joint.orientation.z) == 8)
          compute_quat_w(joint.orientation);
      }
    }
    else if (std::strncmp(buff, "mesh {", 6) == 0) {
      md5_mesh_data_t& data = m.meshes[curr_mesh].data;
      
      int num_verts, num_tris, num_weights = 0;
      int vert_index = 0;
      int tri_index = 0;
      int weight_index = 0;
      float fdata[4];
      int idata[3];
      
      while ((buff[0] != '}') && !std::feof(fp.get())) {
        get_line();
        
        if (std::strstr(buff, "shader ")) {
          const char* sub = std::strtok(buff, "\"");
          sub = std::strtok(nullptr, "\"");
          data.shader = (p.parent_path() / (std::string(sub) + ".tga")).str();
        }
        else if (std::sscanf(buff, " numverts %d", &num_verts) == 1) {
          if (num_verts > 0)
            data.vertices.resize(num_verts);
        }
        else if (std::sscanf(buff, " numtris %d", &num_tris) == 1) {
          if (num_tris > 0)
            data.triangles.resize(num_tris);
        }
        else if (std::sscanf(buff, " numweights %d", &num_weights) == 1) {
          if (num_weights > 0)
            data.weights.resize(num_weights);
        }
        else if (std::sscanf(buff, " vert %d ( %f %f ) %d %d", &vert_index,
                             &fdata[0], &fdata[1], &idata[0], &idata[1]) == 5) {
          data.vertices[vert_index].st.x  = fdata[0];
          data.vertices[vert_index].st.y  = fdata[1];
          data.vertices[vert_index].start = idata[0];
          data.vertices[vert_index].count = idata[1];
        }
        else if (std::sscanf(buff, " tri %d %d %d %d", &tri_index,
                             &idata[0], &idata[1], &idata[2]) == 4) {
          data.triangles[tri_index].index[X] = idata[0];
          data.triangles[tri_index].index[Y] = idata[1];
          data.triangles[tri_index].index[Z] = idata[2];
        }
        else if (std::sscanf(buff, " weight %d %d %f ( %f %f %f )",
                             &weight_index, &idata[0], &fdata[3],
                             &fdata[0], &fdata[1], &fdata[2]) == 6) {
          data.weights[weight_index].joint = idata[0];
          data.weights[weight_index].bias = fdata[3];
          data.weights[weight_index].position = glm::vec3(fdata[0], fdata[1], fdata[2]);
        }
      }
      
      md5_mesh_t& mesh = m.meshes[curr_mesh];
      mesh.vbo.resize(MAX_BUFFER);
      load(mesh.texture, data.shader);
      
      mesh.vertices.resize(data.vertices.size());
      mesh.st.resize(data.vertices.size());
      mesh.normals.resize(data.vertices.size());
      mesh.indices.resize(data.triangles.size() * 3);
      for (int i = 0; i < data.triangles.size(); ++i) {
        mesh.indices[ i * 3]      = data.triangles[i].index[Z];
        mesh.indices[(i * 3) + 1] = data.triangles[i].index[Y];
        mesh.indices[(i * 3) + 2] = data.triangles[i].index[X];
      }
      mesh.weights.resize(data.vertices.size());
      for (int i = 0; i < data.vertices.size(); ++i) {
        mesh.weights[i].x = static_cast<float>(data.vertices[i].count);
        mesh.weights[i].y = static_cast<float>(data.vertices[i].start + index_start);
      }
      index_start += num_weights;
      
      generate_vertex_array(mesh.vao);
      bind(mesh.vao, [&]() {
        generate_array_buffer(mesh.vbo[VERTEX_BUFFER]);
        bind_data<glm::vec3, GL_ARRAY_BUFFER>(mesh.vbo[VERTEX_BUFFER], mesh.vertices);
        glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(VERTEX_BUFFER);
        
        generate_array_buffer(mesh.vbo[TEXTURE_BUFFER]);
        bind_data<glm::vec2, GL_ARRAY_BUFFER>(mesh.vbo[TEXTURE_BUFFER], mesh.st);
        glVertexAttribPointer(TEXTURE_BUFFER, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(TEXTURE_BUFFER);
        
        generate_array_buffer(mesh.vbo[INDEX_BUFFER]);
        bind_data<unsigned int, GL_ELEMENT_ARRAY_BUFFER>(mesh.vbo[INDEX_BUFFER], mesh.indices);
        
        generate_array_buffer(mesh.vbo[NORMAL_BUFFER]);
        bind_data<glm::vec3, GL_ARRAY_BUFFER>(mesh.vbo[NORMAL_BUFFER], mesh.normals);
        glVertexAttribPointer(NORMAL_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(NORMAL_BUFFER);
        
        generate_array_buffer(mesh.vbo[MAX_BUFFER]);
        bind_data<glm::vec2, GL_ARRAY_BUFFER>(mesh.vbo[MAX_BUFFER], mesh.weights);
        glVertexAttribPointer(MAX_BUFFER, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(MAX_BUFFER);
      });
      
      md5_skin_mesh(mesh, m.base_skeleton);
      curr_mesh++;
    }
  }
  
  load_anim(m.animations, ts...);
}

void draw(md5_mesh_t& mesh) {
  bind(mesh.vao, [&]() {
    bind(mesh.texture);
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
  });
}

void draw(md5_t& model) {
  for (auto& mesh: model.meshes)
    draw(mesh);
}

class game: public base {
  shader_t test;
  md5_t model;
  glm::mat4 m;
  
public:
  void init() {
    assets = "/Users/roryb/Documents/git/quick_gl/res";
    
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    m = glm::mat4();
    
    load(test, assets / "test.vert.glsl", assets / "test.frag.glsl");
    
    load(model, assets / "hellknight.md5mesh", assets / "walk7.md5anim");
  }
  
  void tick() {
    if (keyboard.is_key_down(KEY_W))
      move(camera, FORWARD);
    if (keyboard.is_key_down(KEY_A))
      move(camera, LEFT);
    if (keyboard.is_key_down(KEY_S))
      move(camera, BACK);
    if (keyboard.is_key_down(KEY_D))
      move(camera, RIGHT);
    if (keyboard.is_key_down(KEY_Q))
      move(camera, UP);
    if (keyboard.is_key_down(KEY_E))
      move(camera, DOWN);
    
    if (keyboard.is_key_down(KEY_B))
      std::cout << "BREAK" << std::endl;
    
    look(camera, mouse.rel());
    
    // m = glm::rotate(m, glm::radians(5.f), glm::vec3(0.f, 1.f, 0.f));
  }
  
  void render(const float& interpol) {
    use(test, [&]() {
      set_uniform("projection", perspective.mat);
      set_uniform("view", camera.mat);
      set_uniform("model", m);
      draw(model);
    });
  }
};

START_GAME(game /*, 640, 480, WINDOW_FLAG::RESIZABLE*/)
