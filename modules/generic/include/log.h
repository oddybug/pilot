#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
#define NULL_TERMINATOR nullptr
#else
#define NULL_TERMINATOR NULL
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

#ifdef NDEBUG

#define ERROR(...)

#define WARN(...)

#define INFO(...)

#define DEBUG(...)

#define TRACE(...)

#else

void gen_log(const char *level, const char *file, const char *function,
             u32 line, ...);

#define ERROR(...)                                                             \
  gen_log("ERROR", __FILE__, __func__, __LINE__, __VA_ARGS__, NULL_TERMINATOR)

#define WARN(...)                                                              \
  gen_log("WARN", __FILE__, __func__, __LINE__, __VA_ARGS__, NULL_TERMINATOR)

#define INFO(...)                                                              \
  gen_log("INFO", __FILE__, __func__, __LINE__, __VA_ARGS__, NULL_TERMINATOR)

#define DEBUG(...)                                                             \
  gen_log("DEBUG", __FILE__, __func__, __LINE__, __VA_ARGS__, NULL_TERMINATOR)

#define TRACE(...)                                                             \
  gen_log("TRACE", __FILE__, __func__, __LINE__, __VA_ARGS__, NULL_TERMINATOR)

#endif // !NDEBUG

#undef NULL_TERMINATOR

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LOG_H
