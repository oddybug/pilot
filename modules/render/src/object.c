#include <glad/gl.h>

#include <data/atom.h>
#include <data/serial.h>

#include "object.h"
#include <assert.h>

#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"

#include "log.h"

static f32 cube_v[] = {
    // Back face (6 vertices)
    -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    // Front face
    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
    // Left face
    -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
    // Right face
    0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
    // Top face
    -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f};

static f32 cube_n[] = {// Back
                       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
                       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
                       // Front
                       0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                       0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                       // Left
                       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                       // Right
                       1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                       1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                       // Bottom
                       0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                       0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                       // Top
                       0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

// tm stands for texture mappung
static f32 cube_tm[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};

static f32 plane_v[] = {-0.5f, 0.0f, -0.5f, 0.5f,  0.0f, -0.5f,
                        0.5f,  0.0f, 0.5f,  0.5f,  0.0f, 0.5f,
                        -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f, -0.5f};

static f32 plane_n[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

// tm stands for texture mapping
static f32 plane_tm[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};

static f32 hud_plane_v[] = {-1.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f,
                            1.0f,  -1.0f, 0.0f, -1.0f, 1.0f,  0.0f,
                            1.0f,  -1.0f, 0.0f, 1.0f,  1.0f,  0.0f};

static f32 hud_plane_n[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

// tm stands for texture mapping
static f32 hud_plane_tm[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

struct object_T objects[MAX_OBJECTS];

static struct serial_T *serial;

// We are not reusing the VAO and VBO for primitives. Also more advanced
// rendering techinques can be considered to (instancing).
s32 ren_primitive_create_cube(const c8 *name) {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  objects[id].n_triangles = 12;
  objects[id].n_vertices = 36;
  objects[id].n_indices = 0;
  objects[id].primitive = GL_TRIANGLES;
  objects[id].EBO = 0;
  objects[id].indexed = 0;

  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);
  glm_vec3_one(objects[id].scale);
  objects[id].name = name && name[0] ? gen_atom(name) : NULL;

  glGenVertexArrays(1, &objects[id].VAO);
  glBindVertexArray(objects[id].VAO);

  glGenBuffers(3, objects[id].VBO);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_v), cube_v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_n), cube_n, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tm), cube_tm, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  return id;
};

s32 ren_primitive_create_hud_plane(const c8 *name) {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  objects[id].n_triangles = 2;
  objects[id].n_vertices = 6;
  objects[id].n_indices = 0;
  objects[id].primitive = GL_TRIANGLES;
  objects[id].EBO = 0;
  objects[id].indexed = 0;

  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);
  glm_vec3_one(objects[id].scale);
  objects[id].name = name && name[0] ? gen_atom(name) : NULL;

  glGenVertexArrays(1, &objects[id].VAO);
  glBindVertexArray(objects[id].VAO);

  glGenBuffers(3, objects[id].VBO);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(hud_plane_v), hud_plane_v,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(hud_plane_n), hud_plane_n,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(hud_plane_tm), hud_plane_tm,
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  return id;
}
s32 ren_primitive_create_plane(const c8 *name) {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }
  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;
  objects[id].n_triangles = 2;
  objects[id].n_vertices = 6;
  objects[id].n_indices = 0;
  objects[id].primitive = GL_TRIANGLES;
  objects[id].EBO = 0;
  objects[id].indexed = 0;
  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);
  glm_vec3_one(objects[id].scale);
  objects[id].name = name && name[0] ? gen_atom(name) : NULL;
  glGenVertexArrays(1, &objects[id].VAO);
  glBindVertexArray(objects[id].VAO);
  glGenBuffers(3, objects[id].VBO);
  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plane_v), plane_v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plane_n), plane_n, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plane_tm), plane_tm, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);
  return id;
}

s32 ren_primitive_create_line(const c8 *name, vec3 a, vec3 b) {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }
  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;
  objects[id].n_triangles = 0;
  objects[id].n_vertices = 2;
  objects[id].n_indices = 2;
  objects[id].primitive = GL_LINES;
  objects[id].EBO = 0;
  objects[id].indexed = 0;
  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);
  glm_vec3_one(objects[id].scale);
  objects[id].name = name && name[0] ? gen_atom(name) : NULL;
  objects[id].VBO[0] = 0;
  objects[id].VBO[1] = 0;
  objects[id].VBO[2] = 0;
  f32 verts[6] = {a[0], a[1], a[2], b[0], b[1], b[2]};
  u32 idx[2] = {0, 1};
  glGenVertexArrays(1, &objects[id].VAO);
  glBindVertexArray(objects[id].VAO);
  glGenBuffers(1, &objects[id].VBO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, objects[id].VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
  glEnableVertexAttribArray(0);
  glGenBuffers(1, &objects[id].EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[id].EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
  objects[id].indexed = 1;
  glBindVertexArray(0);
  return id;
}

void ren_get_model_mat(s32 id, mat4 model) {

  glm_mat4_identity(model);

  glm_translate(model, objects[id].position);

  glm_rotate(model, objects[id].rotation[0], (vec3){1.0, 0.0, 0.0});
  glm_rotate(model, objects[id].rotation[1], (vec3){0.0, 1.0, 0.0});
  glm_rotate(model, objects[id].rotation[2], (vec3){0.0, 0.0, 1.0});
  glm_scale(model, objects[id].scale);
}

// model[0] = objects[i]; };
