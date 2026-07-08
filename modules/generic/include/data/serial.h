#ifndef SERIAL_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define T serial_T

struct T {
  u32 current;
};

/**
 * @brief creates a serial. The first serial number stamped will be 0. User must
 * call gen_serial_delete do free serial in memory.
 *
 * @return the serial or NULLPTR if failed. Object is allocated in the heap
 */
struct T *gen_serial_create(void);

/**
 * @brief creates a serial. The first serial number stamped will be i_stamp.
 * User must call gen_serial_delete do free serial in memory.
 *
 * @param i_stamp
 * @return
 */
struct T *gen_serial_create_from(s32 i_stamp);

/**
 * @brief returns the next serial number availible in 'sn'.
 *
 * @param serial
 * @param sn
 * @return 0 succes and 1 if reached max serial numbers generated.
 */
s32 gen_serial_stamp(struct T *serial, s32 *sn);

/**
 * @brief checks if serial number 'n' exists in the serial 'serial'
 *
 * @param serial
 * @param n
 * @return 0 does not exists, 1 exists
 */
s32 gen_serial_exists(struct T *serial, s32 n);

/**
 * @brief destroy serial 'serial'
 *
 * @param serial
 * @return 0 on succes and 1 on error.
 */
s32 gen_serial_destroy(struct T *serial);

#undef T

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !SERIAL_H
