#ifndef ATOM_H
#define ATOM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

const c8 *gen_atom(const c8 *str);

void gen_atom_free(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !ATOM_H
