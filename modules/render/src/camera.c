#include "camera.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"

#include <math.h>

#define ORBIT_PITCH_LIM 1.5508f
#define ORBIT_RADIUS_MIN 0.05f

// mat4 _view_matrix;
// mat4 _projection_matrix;

struct camera_T main_camera;

void res_init_camera() {
  glm_vec3_zero(main_camera.position);
  glm_vec3_zero(main_camera.front);
  glm_vec3_zero(main_camera.euler_angles);
  glm_vec3_zero(main_camera.orbit_angles);
  glm_vec3_zero(main_camera.orbit_origin);
  ren_camera_orbit_r(2.0f);
  main_camera.mode = CAMERA_FPV;

  // glm_mat4_zero(_view_matrix);
  // glm_mat4_zero(_projection_matrix);
}

static void forward_from_angles_(vec3 e, vec3 out) {
  f32 s0 = sinf(e[0]);
  f32 c0 = cosf(e[0]);
  f32 s1 = sinf(e[1]);
  f32 c1 = cosf(e[1]);
  f32 s2 = sinf(e[2]);
  f32 c2 = cosf(e[2]);
  f32 x = -c0 * s1;
  f32 y = s0;
  f32 z = -c0 * c1;
  out[0] = x * c2 - y * s2;
  out[1] = x * s2 + y * c2;
  out[2] = z;
}

static void orbit_position_(vec3 out) {
  vec3 fwd;
  forward_from_angles_(main_camera.orbit_angles, fwd);
  out[0] = main_camera.orbit_origin[0] - fwd[0] * main_camera.orbit_radius;
  out[1] = main_camera.orbit_origin[1] - fwd[1] * main_camera.orbit_radius;
  out[2] = main_camera.orbit_origin[2] - fwd[2] * main_camera.orbit_radius;
}

static void sync_front_() {
  if (main_camera.mode == CAMERA_ORBIT) {
    forward_from_angles_(main_camera.orbit_angles, main_camera.front);
  } else {
    forward_from_angles_(main_camera.euler_angles, main_camera.front);
  }
}

static void clamp_orbit_pitch_() {
  if (main_camera.orbit_angles[0] > ORBIT_PITCH_LIM) {
    main_camera.orbit_angles[0] = ORBIT_PITCH_LIM;
  } else if (main_camera.orbit_angles[0] < -ORBIT_PITCH_LIM) {
    main_camera.orbit_angles[0] = -ORBIT_PITCH_LIM;
  }
}

static void build_view_(mat4 out_view, vec3 angles, vec3 pos) {
  glm_mat4_identity(out_view);

  glm_rotate(out_view, -angles[0], (vec3){1.0, 0.0, 0.0});
  glm_rotate(out_view, -angles[1], (vec3){0.0, 1.0, 0.0});
  glm_rotate(out_view, -angles[2], (vec3){0.0, 0.0, 1.0});

  vec3 negative_pos;
  glm_vec3_negate_to(pos, negative_pos);
  glm_translate(out_view, negative_pos);
}

void ren_rotate_camera(vec3 euler_angles) {
  glm_vec3_add(main_camera.euler_angles, euler_angles,
               main_camera.euler_angles);
  sync_front_();
};

void ren_translate_camera(vec3 translation) {
  if (main_camera.mode == CAMERA_ORBIT) {
    glm_vec3_add(main_camera.orbit_origin, translation,
                 main_camera.orbit_origin);
  } else {
    glm_vec3_add(main_camera.position, translation, main_camera.position);
  }
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

void ren_get_view_matrix(mat4 out_view) {
  if (main_camera.mode == CAMERA_ORBIT) {
    vec3 pos;
    orbit_position_(pos);
    build_view_(out_view, main_camera.orbit_angles, pos);
    return;
  }

  build_view_(out_view, main_camera.euler_angles, main_camera.position);
}

void ren_get_projection_matrix(mat4 *out_projection) {
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

void ren_camera_set_position(f32 x, f32 y, f32 z) {
  main_camera.position[0] = x;
  main_camera.position[1] = y;
  main_camera.position[2] = z;
  if (main_camera.mode == CAMERA_ORBIT) {
    vec3 fwd;
    forward_from_angles_(main_camera.orbit_angles, fwd);
    main_camera.orbit_origin[0] = x + fwd[0] * main_camera.orbit_radius;
    main_camera.orbit_origin[1] = y + fwd[1] * main_camera.orbit_radius;
    main_camera.orbit_origin[2] = z + fwd[2] * main_camera.orbit_radius;
  }
};

void ren_camera_set_rotation(f32 yaw, f32 pitch, f32 roll) {
  main_camera.euler_angles[0] = yaw;
  main_camera.euler_angles[1] = pitch;
  main_camera.euler_angles[2] = roll;
  sync_front_();
};

void ren_camera_set_position_v(vec3 pos) {
  main_camera.position[0] = pos[0];
  main_camera.position[1] = pos[1];
  main_camera.position[2] = pos[2];
  if (main_camera.mode == CAMERA_ORBIT) {
    vec3 fwd;
    forward_from_angles_(main_camera.orbit_angles, fwd);
    main_camera.orbit_origin[0] = pos[0] + fwd[0] * main_camera.orbit_radius;
    main_camera.orbit_origin[1] = pos[1] + fwd[1] * main_camera.orbit_radius;
    main_camera.orbit_origin[2] = pos[2] + fwd[2] * main_camera.orbit_radius;
  }
}

void ren_camera_set_rotation_v(vec3 euler_angles) {
  main_camera.euler_angles[0] = euler_angles[0];
  main_camera.euler_angles[1] = euler_angles[1];
  main_camera.euler_angles[2] = euler_angles[2];
  sync_front_();
}

void ren_camera_mode(enum CAMERA_MODE mode) {
  if (main_camera.mode == mode) {
    return;
  }
  if (mode == CAMERA_ORBIT) {
    glm_vec3_copy(main_camera.euler_angles, main_camera.orbit_angles);
    vec3 fwd;
    forward_from_angles_(main_camera.euler_angles, fwd);
    main_camera.orbit_origin[0] =
        main_camera.position[0] + fwd[0] * main_camera.orbit_radius;
    main_camera.orbit_origin[1] =
        main_camera.position[1] + fwd[1] * main_camera.orbit_radius;
    main_camera.orbit_origin[2] =
        main_camera.position[2] + fwd[2] * main_camera.orbit_radius;
  } else {
    vec3 pos;
    orbit_position_(pos);
    glm_vec3_copy(pos, main_camera.position);
    glm_vec3_copy(main_camera.orbit_angles, main_camera.euler_angles);
  }
  main_camera.mode = mode;
  sync_front_();
}

void ren_camera_orbit(vec3 delta_angles) {
  glm_vec3_add(main_camera.orbit_angles, delta_angles,
               main_camera.orbit_angles);
  clamp_orbit_pitch_();
  sync_front_();
}

void ren_camera_orbit_o(vec3 origin) {
  glm_vec3_copy(origin, main_camera.orbit_origin);
}

void ren_camera_orbit_a(vec3 angles) {
  glm_vec3_copy(angles, main_camera.orbit_angles);
  clamp_orbit_pitch_();
  sync_front_();
}

void ren_camera_orbit_r(f32 radius) {
  main_camera.orbit_radius =
      radius < ORBIT_RADIUS_MIN ? ORBIT_RADIUS_MIN : radius;
}

f32 ren_camera_orbit_get_r(void) { return main_camera.orbit_radius; }

f32 ren_camera_orbit_get_yaw(void) { return main_camera.orbit_angles[1]; }
