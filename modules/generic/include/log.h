#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
#define NULL_TERMINATOR nullptr
#else
#define NULL_TERMINATOR NULL
#endif // __cplusplus

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

#define ERROR(fmt, ...)                                                        \
  gen_log("ERROR", __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)                                                         \
  gen_log("WARN", __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)                                                         \
  gen_log("INFO", __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)                                                        \
  gen_log("DEBUG", __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define TRACE(fmt, ...)                                                        \
  gen_log("TRACE", __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

#endif // !NDEBUG

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LOG_H
