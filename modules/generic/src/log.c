#ifndef NDEBUG

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define GRAY "\033[90m"
#define GREEN "\033[32m"
#define PURPLE "\033[35m"
#define PURPLE_BACKGROUND "\033[45m"
#define RED "\033[31m"
#define YELLOW "\033[33m"

#define BOLD "\033[1m"
#define RESET "\033[0m"

#define N_LEVELS 5
enum { TRACE = 0, DEBUG, INFO, WARN, ERROR };
static const char *levels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};

/**
 * @brief compares string 'level' with all 'levels' entries and return the index
 * of the matched entry.
 *
 * @param level
 * @return match index entry or -1 if not found.
 */
static s32 gen_parse_level(const char *level) {

  for (u32 i = 0; i < N_LEVELS; ++i) {
    if (strcmp(levels[i], level) == 0) {
      return i;
    }
  }
  return -1;
}

static void gen_write_log(u32 level, const char *file, const char *function,
                          u32 line, va_list list) {

  const char *color = RESET;

  switch (level) {
  case TRACE:
    color = CYAN;
    break;
  case DEBUG:
    color = GREEN;
    break;
  case INFO:
    color = BLUE;
    break;
  case WARN:
    color = YELLOW;
    break;
  case ERROR:
    color = RED;
    break;
  default:
    break;
  }

  fprintf(stderr, "%s%s[%s]%s(%s:%s:%u) -", color, BOLD, levels[level], RESET,
          file, function, line);

  const char *msg = va_arg(list, const char *);

  while (msg != NULL) {
    fprintf(stderr, " %s", msg);
    msg = va_arg(list, const char *);
  }

  fprintf(stderr, "\n");
}

void gen_log(const char *level, const char *file, const char *function,
             u32 line, ...) {

  const char *env_level = getenv("LOG_LEVEL");

  if (!env_level)
    return;

  u32 env_parsed = gen_parse_level(env_level);

  if (env_parsed == -1)
    return;

  u32 req_parsed = gen_parse_level(level);

  if (req_parsed == -1)
    return;

  if (!(req_parsed >= env_parsed))
    return;

  va_list args;
  va_start(args, line);
  gen_write_log(req_parsed, file, function, line, args);
  va_end(args);
};

#undef BLUE
#undef CYAN
#undef GRAY
#undef GREEN
#undef PURPLE
#undef PURPLE_BACKGROUND
#undef RED
#undef YELLOW

#undef BOLD
#undef RESET

#undef N_LEVELS

#endif /* ifdef NDEBUG */
