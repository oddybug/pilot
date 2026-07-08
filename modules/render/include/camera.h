#ifdef __cplusplus
extern "C" {
#endif

#include <cglm/cglm.h>
#include <types.h>

struct camera_T {
  vec3 position;
  vec3 front;
  vec3 euler_angles;

  f32 fov;
  f32 aspect_ratio;
  f32 near_plane;
  f32 far_plane;

  u8 projectio_type;
};

enum CAMERA_PROJECTION { ORTOGONAL = 0, PERSPECTIVE };

extern struct camera_T main_camera;

// extern struct camera_T main_camera;

/**
 * @brief initializes camera values to 0
 */
extern void res_init_camera();

/**
 * @brief rotate the looking direction of camera. Pivot point is relative to
 * camera.
 *
 * @param euler_angles
 */
extern void ren_rotate_camera(vec3 euler_angles);

/**
 * @brief adds translaiton to camera position. Translation relative to camera.
 *
 * @param translation
 */
extern void ren_translate_camera(vec3 translation);

/**
 * @brief set camera fov
 *
 * @param fov
 */
extern void ren_set_camera_fov(f32 fov);

/**
 * @brief set near and back planes of camera fustrum
 *
 * @param near
 * @param far
 */
extern void ren_set_camera_planes(f32 near, f32 far);

/**
 * @brief Sets camera projection. See CAMERA_PROJECTION for available modes.
 *
 * @param projection: see enum CAMERA_PROJECTION for values
 */
extern void ren_set_camera_projection(s32 projection);

/**
 * @brief set camera aspect ratio
 *
 * @param ratio
 */
extern void ren_set_camera_aspect_ratio(f32 ratio);

/**
 * @brief set camera aspect ratio with width and height
 *
 * @param width
 * @param height
 */
extern void ren_set_camera_aspect_ratio_wh(f32 width, f32 height);

/**
 * @brief returns the view matrix of the cmaera in out_view
 *
 * @param out_view
 * @return
 */
extern void ren_get_view_matrix(mat4 out_view);

/**
 * @brief return projection matrix depending on enum CAMERA_PROJECTION. Result
 * is stored in out_projection
 *
 * @param out_projection
 */
extern void ren_get_projection_matrix(mat4 *out_projection);

extern void ren_camera_set_position(f32 x, f32 y, f32 z);

extern void ren_camera_set_rotation(f32 yaw, f32 pitch, f32 roll);


extern void ren_camera_set_position_v(vec3 pos);

extern void ren_camera_set_rotation_v(vec3 euler_angles);

#ifdef __cplusplus
}
#endif
