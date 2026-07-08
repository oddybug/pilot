#include "camera.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"

// mat4 _view_matrix;
// mat4 _projection_matrix;

struct camera_T main_camera;

void res_init_camera() {
  glm_vec3_zero(main_camera.position);
  glm_vec3_zero(main_camera.front);
  glm_vec3_zero(main_camera.euler_angles);

  // glm_mat4_zero(_view_matrix);
  // glm_mat4_zero(_projection_matrix);
}

void ren_rotate_camera(vec3 euler_angles) {
  glm_vec3_add(main_camera.euler_angles, euler_angles,
               main_camera.euler_angles);
};

void ren_translate_camera(vec3 translation) {
  glm_vec3_add(main_camera.position, translation, main_camera.position);
};

void ren_set_camera_fov(f32 fov) { main_camera.fov = fov; };

void ren_set_camera_planes(f32 near, f32 far) {
  main_camera.near_plane = near;
  main_camera.far_plane = far;
};

void ren_set_camera_projection(s32 projection) {
  main_camera.projectio_type = projection;
};

void ren_set_camera_aspect_ratio(f32 ratio) {
  main_camera.aspect_ratio = ratio;
};

void ren_set_camera_aspect_ratio_wh(f32 width, f32 height) {
  main_camera.aspect_ratio = width / height;
};

extern void ren_get_view_matrix(mat4 out_view) {
  glm_mat4_identity(out_view);

  glm_rotate(out_view, -main_camera.euler_angles[2], (vec3){0.0, 0.0, 1.0});
  glm_rotate(out_view, -main_camera.euler_angles[1], (vec3){0.0, 1.0, 0.0});
  glm_rotate(out_view, -main_camera.euler_angles[0], (vec3){1.0, 0.0, 0.0});

  vec3 negative_pos;
  glm_vec3_negate_to(main_camera.position, negative_pos);
  glm_translate(out_view, negative_pos);
}

extern void ren_get_projection_matrix(mat4 *out_projection) {
  switch (main_camera.projectio_type) {
  case PERSPECTIVE:
    glm_perspective(main_camera.fov, main_camera.aspect_ratio,
                    main_camera.near_plane, main_camera.far_plane,
                    *out_projection);
    break;
  case ORTOGONAL:
    // TODO: adding ortogonal support
    break;
  default:
    break;
  }
};

extern void ren_camera_set_position(f32 x, f32 y, f32 z) {
  main_camera.position[0] = x;
  main_camera.position[1] = y;
  main_camera.position[2] = z;
};

extern void ren_camera_set_rotation(f32 yaw, f32 pitch, f32 roll) {
  main_camera.euler_angles[0] = yaw;
  main_camera.euler_angles[1] = pitch;
  main_camera.euler_angles[2] = roll;
};

extern void ren_camera_set_position_v(vec3 pos) {
  main_camera.position[0] = pos[0];
  main_camera.position[1] = pos[1];
  main_camera.position[2] = pos[2];
}

extern void ren_camera_set_rotation_v(vec3 euler_angles) {
  main_camera.euler_angles[0] = euler_angles[0];
  main_camera.euler_angles[1] = euler_angles[1];
  main_camera.euler_angles[2] = euler_angles[2];
}
