#pragma once

// This allocator is to be used for resources which will exist for the entire 
// period the program is running
struct PermanentResourceAllocator {
  i64 size;
  i64 offset;
  uchar* backing_buffer;

  PermanentResourceAllocator(i64 size) {
    this->size = size;
    this->offset = 0;

    this->backing_buffer = (uchar*)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  }

  void* Allocate(i64 alloc_size) {
    if (offset + alloc_size < size) {
      void* temp = &backing_buffer[offset];
      offset += alloc_size;
      memset(temp, 0, alloc_size);
      return temp;
    }
    return NULL;
  }

  void FreeAll() {
    if (backing_buffer) {
      VirtualFree(backing_buffer, 0, MEM_RELEASE);
    }
    size = 0;
    offset = 0;
  }
};

struct Vector2 {
  float x, y;
};

struct Vector3 {
  float x, y, z;
};

struct int3 {
  int v, n, t;
};

struct Face {
  int3 f[3];
};

Vector3 vertices[1024];
Vector3 normals[1024];
Vector2 texcoords[1024];
Face faces[1024];

int vi = 0;
int ni = 0;
int ti = 0;

int fi = 0;

int num_vertices_and_indices = 0;

struct Vertices {
  Vector3* pos;
  Vector3* norm;
  Vector2* tex;
};

Vertices final_vertices;
int* final_indices;

dev_ReadFileResult obj_file;

void LoadOBJModel(char* model_path, PermanentResourceAllocator* allocator) {
  obj_file = dev_ReadFile(model_path);

  uchar* ptr = (uchar*)obj_file.data;
  
  int i = 0;
  while (i < obj_file.size) {
    if (ptr[i] == 'f') {
      num_vertices_and_indices += 1;
    }
    while (ptr[i] != '\n') { i += 1; }
    i += 1;
  }
  num_vertices_and_indices *= 3;

  // allocate enough space for num_vertices_and_indices in the final_vertices and final_indices
  final_vertices.pos = (Vector3*)allocator->Allocate(sizeof(Vector3) * num_vertices_and_indices);
  final_vertices.norm = (Vector3*)allocator->Allocate(sizeof(Vector3) * num_vertices_and_indices);
  final_vertices.tex = (Vector2*)allocator->Allocate(sizeof(Vector2) * num_vertices_and_indices);
  final_indices = (int*)allocator->Allocate(sizeof(int) * num_vertices_and_indices);

  i = 0;
  while (i < obj_file.size) {
    if (ptr[i] == '#' || ptr[i] == ' ') { // skip lines which have the comment operator #, and skip blank lines
      while (ptr[i] != '\n') { i += 1; }
      i += 1;
    }
    else if (ptr[i] == 'm') { // we ignore materials for now
      while (ptr[i] != '\n') { i += 1; }
      i += 1;
    }
    else if (ptr[i] == 's' || ptr[i] == 'o' || ptr[i] == 'g' || ptr[i] == 'u') { // ignore soft shading, objects, groups, and usematerial for now
      while (ptr[i] != '\n') { i += 1; }
      i += 1;
    }
    else if (ptr[i] == 'v') { // we found vertex information
      i += 1;
      if (ptr[i] == ' ') { // we found a vertex
        i += 1;
        vertices[vi].x = (float)atof((char*)&ptr[i]);
        while (ptr[i] != ' ') {
          i += 1;
        }
        i += 1;
        vertices[vi].y = (float)atof((char*)&ptr[i]);
        while (ptr[i] != ' ') {
          i += 1;
        }
        i += 1;
        vertices[vi].z = (float)atof((char*)&ptr[i]);
        while (ptr[i] != '\n') {
          i += 1;
        }
        i += 1;

        vi += 1;
      }
      else if (ptr[i] == 't') { // we found a texcoord
        i += 2;
        texcoords[ti].x = (float)atof((char*)&ptr[i]);
        while (ptr[i] != ' ') {
          i += 1;
        }
        i += 1;
        texcoords[ti].y = (float)atof((char*)&ptr[i]);
        while (ptr[i] != '\n') {
          i += 1;
        }
        i += 1;

        ti += 1;
      }
      else if (ptr[i] == 'n') { // we found a normal
        i += 2;
        normals[ni].x = (float)atof((char*)&ptr[i]);
        while (ptr[i] != ' ') {
          i += 1;
        }
        i += 1;
        normals[ni].y = (float)atof((char*)&ptr[i]);
        while (ptr[i] != ' ') {
          i += 1;
        }
        i += 1;
        normals[ni].z = (float)atof((char*)&ptr[i]);
        while (ptr[i] != '\n') {
          i += 1;
        }
        i += 1;

        ni += 1;
      }
      else if (ptr[i] == 'p') { // ignore parametric curve stuff
        while (ptr[i] != '\n') { i += 1; }
        i += 1;
      }
    }
    else if (ptr[i] == 'f') { // faces
      i += 2;
      faces[fi].f[0].v = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[0].t = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[0].n = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != ' ') { i += 1; }
      i += 1;

      faces[fi].f[1].v = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[1].t = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[1].n = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != ' ') { i += 1; }
      i += 1;

      faces[fi].f[2].v = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[2].t = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '/') { i += 1; }
      i += 1;
      faces[fi].f[2].n = atoi((char*)&ptr[i]) - 1;
      while (ptr[i] != '\n') { i += 1; }
      i += 1;
      fi += 1;
    }
  }

  i = 0;
  fi = 0;
  int indices_index = 0;
  for (fi = 0; fi < 1024; fi++) { // compose vertices 
    int index = 3 * fi;
    
    final_vertices.pos[index+0]    = vertices[faces[fi].f[0].v];
    final_vertices.norm[index + 0] = normals[faces[fi].f[0].n];
    final_vertices.tex[index + 0]  = texcoords[faces[fi].f[0].t];
    final_indices[indices_index] = indices_index;
    indices_index += 1;

    final_vertices.pos[index + 1]  = vertices[faces[fi].f[1].v];
    final_vertices.norm[index + 1] = normals[faces[fi].f[1].n];
    final_vertices.tex[index + 1]  = texcoords[faces[fi].f[1].t];
    final_indices[indices_index] = indices_index;
    indices_index += 1;

    final_vertices.pos[index + 2]  = vertices[faces[fi].f[2].v];
    final_vertices.norm[index + 2] = normals[faces[fi].f[2].n];
    final_vertices.tex[index + 2]  = texcoords[faces[fi].f[2].t];
    final_indices[indices_index] = indices_index;
    indices_index += 1;
  }

  OutputDebugStringA("Hello\n");
}