#include <glad/gl.h>

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

static f32 cube_n[] = { // Back
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    // Front
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    // Left
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    // Right
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    // Bottom
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    // Top
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

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

s32 ren_primitive_create_cube() {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  objects[id].n_triangles = 12;

  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);

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

s32 ren_primitive_create_hud_plane() {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  objects[id].n_triangles = 2;

  glm_vec3_zero(objects[id].position);
  glm_vec3_zero(objects[id].rotation);

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
void ren_get_model_mat(s32 id, mat4 model) {

  glm_mat4_identity(model);

  glm_translate(model, objects[id].position);

  glm_rotate(model, objects[id].rotation[0], (vec3){1.0, 0.0, 0.0});
  glm_rotate(model, objects[id].rotation[1], (vec3){0.0, 1.0, 0.0});
  glm_rotate(model, objects[id].rotation[2], (vec3){0.0, 0.0, 1.0});
}

// model[0] = objects[i]; };
