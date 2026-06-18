#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include <camera.h>

// mat4 _view_matrix;
// mat4 _projection_matrix;

struct camera {
  vec3 position;
  vec3 front;
  vec3 euler_angles;

  f32 fov;
  f32 aspect_ratio;
  f32 near_plane;
  f32 far_plane;

  u8 projectio_type;
};

struct camera _camera;

void res_init_camera() {
  glm_vec3_zero(_camera.position);
  glm_vec3_zero(_camera.front);
  glm_vec3_zero(_camera.euler_angles);

  // glm_mat4_zero(_view_matrix);
  // glm_mat4_zero(_projection_matrix);
}

void ren_rotate_camera(vec3 euler_angles) {
  glm_vec3_add(_camera.euler_angles, euler_angles, _camera.euler_angles);
};

void ren_translate_camera(vec3 translation) {
  glm_vec3_add(_camera.position, translation, _camera.position);
};

void ren_set_camera_fov(f32 fov) { _camera.fov = fov; };

void ren_set_camera_planes(f32 near, f32 far) {
  _camera.near_plane = near;
  _camera.far_plane = far;
};

void ren_set_camera_projection(s32 projection) {
  _camera.projectio_type = projection;
};

void ren_set_camera_aspect_ratio(f32 ratio) { _camera.aspect_ratio = ratio; };

void ren_set_camera_aspect_ratio_wh(f32 width, f32 height) {
  _camera.aspect_ratio = width / height;
};

/**
 * @brief returns the view matrix of the cmaera in out_view
 *
 * @param out_view
 * @return
 */
void _get_view_matrix(mat4 out_view);

void _get_view_matrix(mat4 out_view) { glm_mat4_zero(out_view); }

/**
 * @brief return projection matrix depending on enum CAMERA_PROJECTION. Result
 * is stored in out_projection
 *
 * @param out_projection
 */
void _get_projection_matrix(mat4 out_projection);

void _get_projection_matrix(mat4 out_projection) {
  switch (_camera.projectio_type) {
  case PERSPECTIVE:
    glm_perspective(_camera.fov, _camera.aspect_ratio, _camera.near_plane,
                    _camera.far_plane, out_projection);
    break;
  case ORTOGONAL:
    // TODO: adding ortogonal support
    break;
  default:
    break;
  }
};
