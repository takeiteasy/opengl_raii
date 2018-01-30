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

struct md5_mesh_t {
  texture_t texture;
  std::vector<md5_vertex_t> vertices;
  std::vector<md5_triangle_t> triangles;
  std::vector<md5_weight_t> weights;
  GLuint VAO, VBO, EBO;
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

struct md5_anim_t {
  int frame_rate;
  std::vector<std::vector<md5_joint_t>> frames;
  std::vector<md5_bbox_t> bboxes;
};

struct anim_info_t {
  int curr_frame;
  int next_frame;
  
  double last_time;
  double max_time;
};

struct md5_t {
  std::vector<md5_joint_t> base_skel;
  std::vector<md5_mesh_t> meshes;
  std::map<std::string, md5_anim_t> animations;
};

auto compute_quat_w(glm::quat& q) {
  float t = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
  q.w = (t < 0.0f ? 0.0f : -sqrt(t));
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
  std::vector<baseframe_joint_t> baseFrame;
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
        baseFrame.resize(num_joints);
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
                        &baseFrame[i].position.x,    &baseFrame[i].position.y,    &baseFrame[i].position.z,
                        &baseFrame[i].orientation.x, &baseFrame[i].orientation.y, &baseFrame[i].orientation.z) == 6)
          compute_quat_w(baseFrame[i].orientation);
      }
    }
    else if (std::sscanf(buff, " frame %d", &frame_index) == 1) {
      for (int i = 0; i < num_anim_comps; ++i)
        std::fscanf(fp.get(), "%f", &anim_frame_data[i]);
      
      for (int i = 0; i < num_joints; ++i) {
        const baseframe_joint_t& base_joint = baseFrame[i];
        
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
  
  load_anim(m, ts...);
}

template<typename... T, typename=path> auto load(md5_t& m, const path& p, const T& ... ts) {
  if (not p.exists() or not p.is_file())
    throw std::runtime_error("md5mesh path \"" + p.str() + "\" is not a file or doesn't exist");
  
  std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(p.str().c_str(), "r"), &std::fclose);
  if (!fp)
    throw std::runtime_error(p.str());
  
  char buff[512];
  int version, curr_mesh = 0, num_joints, num_meshes;
  const auto get_line = std::bind(std::fgets, buff, sizeof(buff), fp.get());
  
  while (!std::feof(fp.get())) {
    get_line();
    
    if (std::sscanf(buff, " MD5Version %d", &version) == 1) {
      if (version != 10)
        throw std::runtime_error("ERROR: Bad model version");
    }
    else if (std::sscanf(buff, " numJoints %d", &num_joints) == 1) {
      if (num_joints > 0)
        m.base_skel.resize(num_joints);
    }
    else if (std::sscanf(buff, " numMeshes %d", &num_meshes) == 1) {
      if (num_meshes > 0)
        m.meshes.resize(num_meshes);
    }
    else if (std::strncmp(buff, "joints {", 8) == 0) {
      for (int i = 0; i < num_joints; ++i) {
        md5_joint_t* joint = &m.base_skel[i];
        
        get_line();
        if (std::sscanf(buff, "%s %d ( %f %f %f ) ( %f %f %f )",
                         joint->name, &joint->parent,
                         &joint->position[0], &joint->position[1], &joint->position[2],
                         &joint->orientation[0], &joint->orientation[1], &joint->orientation[2]) == 8)
          compute_quat_w(joint->orientation);
      }
    }
    else if (std::strncmp(buff, "mesh {", 6) == 0) {
      md5_mesh_t& mesh = m.meshes[curr_mesh];
      
      int num_verts, num_tris, num_weights;
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
          load(mesh.texture, p.parent_path() / (std::string(sub) + ".tga"));
        }
        else if (std::sscanf(buff, " numverts %d", &num_verts) == 1) {
          if (num_verts > 0)
            mesh.vertices.resize(num_verts);
        }
        else if (std::sscanf(buff, " numtris %d", &num_tris) == 1) {
          if (num_tris > 0)
            mesh.triangles.resize(num_tris);
        }
        else if (std::sscanf(buff, " numweights %d", &num_weights) == 1) {
          if (num_weights > 0)
            mesh.weights.resize(num_weights);
        }
        else if (std::sscanf(buff, " vert %d ( %f %f ) %d %d", &vert_index,
                             &fdata[0], &fdata[1], &idata[0], &idata[1]) == 5) {
          mesh.vertices[vert_index].st.x  = fdata[0];
          mesh.vertices[vert_index].st.y  = fdata[1];
          mesh.vertices[vert_index].start = idata[0];
          mesh.vertices[vert_index].count = idata[1];
        }
        else if (std::sscanf(buff, " tri %d %d %d %d", &tri_index,
                             &idata[0], &idata[1], &idata[2]) == 4) {
          mesh.triangles[tri_index].index[X] = idata[0];
          mesh.triangles[tri_index].index[Y] = idata[1];
          mesh.triangles[tri_index].index[Z] = idata[2];
        }
        else if (std::sscanf(buff, " weight %d %d %f ( %f %f %f )",
                             &weight_index, &idata[0], &fdata[3],
                             &fdata[0], &fdata[1], &fdata[2]) == 6) {
          mesh.weights[weight_index].joint = idata[0];
          mesh.weights[weight_index].bias = fdata[3];
          mesh.weights[weight_index].position = glm::vec3(fdata[0], fdata[1], fdata[2]);
        }
      }
      
      std::vector<vertex_t> tmp_verts;
      tmp_verts.resize(mesh.vertices.size());
      for (int i = 0; i < mesh.vertices.size(); ++i) {
        vertex_t& vert = tmp_verts[i];
        md5_vertex_t& old_vert = mesh.vertices[i];
        int count = old_vert.count;
        for (int j = 0; j < count; ++j) {
          if (j >= 4)
            break;
          
          md5_weight_t& weight = mesh.weights[old_vert.start + j];
          md5_joint_t&  joint  = m.base_skel[weight.joint];
          
          vert.position  += (joint.position + joint.orientation * weight.position) * weight.bias;
          vert.indices[j] = static_cast<float>(weight.joint);
          vert.weights[j] = weight.bias;
        }
        vert.uv = old_vert.st;
      }
      
      for (int i = 0; i < mesh.triangles.size(); ++i) {
        glm::vec3& v = tmp_verts[mesh.triangles[i].index[X]].position;
        glm::vec3 norm = glm::cross(tmp_verts[mesh.triangles[i].index[Z]].position - v,
                                    tmp_verts[mesh.triangles[i].index[Y]].position - v);
        tmp_verts[mesh.triangles[i].index[X]].normal += norm;
        tmp_verts[mesh.triangles[i].index[Y]].normal += norm;
        tmp_verts[mesh.triangles[i].index[Z]].normal += norm;
      }
      
      for (auto& v: tmp_verts)
        v.normal = glm::normalize(v.normal);
      
      glGenVertexArrays(1, &mesh.VAO);
      glGenBuffers(1, &mesh.VBO);
      glGenBuffers(1, &mesh.EBO);
      
      glBindVertexArray(mesh.VAO);
      
      glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
      glBufferData(GL_ARRAY_BUFFER, tmp_verts.size() * sizeof(vertex_t), &tmp_verts[0], GL_STATIC_DRAW);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triangles.size() * 3 * sizeof(md5_triangle_t), &mesh.triangles[0], GL_STATIC_DRAW);
      
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, uv));
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, weights));
      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, indices));
      
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      
      curr_mesh++;
    }
  }
  
  load_anim(m.animations, ts...);
}

void draw(md5_mesh_t& mesh) {
  bind(mesh.texture, [&mesh]() {
    set_uniform<int>("texture_diffuse", 0);
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.triangles.size()) * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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
    
    load(model, assets / "bob_lamp_update.md5mesh", assets / "bob_lamp_update.md5anim");
    
    printf("");
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
