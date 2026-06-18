#ifdef __cplusplus
extern "C" {
#endif

#include <cglm/cglm.h>
#include <types.h>

struct camera;

/**
 * @brief initializes camera values to 0
 */
void res_init_camera();

/**
 * @brief rotate the looking direction of camera. Pivot point is relative to
 * camera.
 *
 * @param euler_angles
 */
void ren_rotate_camera(vec3 euler_angles);

/**
 * @brief adds translaiton to camera position. Translation relative to camera.
 *
 * @param translation
 */
void ren_translate_camera(vec3 translation);

/**
 * @brief set camera fov
 *
 * @param fov
 */
void ren_set_camera_fov(f32 fov);

/**
 * @brief set near and back planes of camera fustrum
 *
 * @param near
 * @param far
 */
void ren_set_camera_planes(f32 near, f32 far);

enum CAMERA_PROJECTION { ORTOGONAL = 0, PERSPECTIVE };

/**
 * @brief Sets camera projection. See CAMERA_PROJECTION for available modes.
 *
 * @param projection: see enum CAMERA_PROJECTION for values
 */
void ren_set_camera_projection(s32 projection);

/**
 * @brief set camera aspect ratio
 *
 * @param ratio
 */
void ren_set_camera_aspect_ratio(f32 ratio);

/**
 * @brief set camera aspect ratio with width and height
 *
 * @param width
 * @param height
 */
void ren_set_camera_aspect_ratio_wh(f32 width, f32 height);

void update_camera();

#ifdef __cplusplus
}
#endif
