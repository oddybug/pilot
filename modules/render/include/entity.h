#ifndef ENTITY_H
#define ENTITY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <types.h>

enum COMPONENT_TYPE {
  COMPONENT_PROGRAM = 0,
  COMPONENT_MATERIAL,
  COMPONENT_OBJECT,
  COMPONENT_COUNT
};

struct entity_T {
  u32 components[COMPONENT_COUNT];
  u32 id;
};

#define MAX_ENTITIES 1024
extern struct entity_T entities[MAX_ENTITIES];

/**
 * @brief It is granted that no entity can have id equals 0
 *
 * @return Returns 0 on failure. Otherwise id of entity.
 */
extern u32 ren_create_entity();

/**
 * @brief returns entity ID. Undefined behaivour when passing NULL pointer.
 *
 * @param entity
 * @return
 */
extern u32 get_entity_id(u32 entity);

extern void ren_entity_add_component(u32 entity, enum COMPONENT_TYPE type,
                                     u32 component);

extern s32 ren_entity_get_component(u32 entity, enum COMPONENT_TYPE type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !ENTITY_H
