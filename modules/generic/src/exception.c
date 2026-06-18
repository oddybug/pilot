#include <stdio.h>
#include <stdlib.h>

// #include "assert.h"
#include "exception.h"

#define T except_T

struct Except_Frame {
  Except_Frame *prev;
  jmp_buf env;
  const char *file;
  int line;
  const T *exception;
};

Except_Frame *Except_stack = NULL;

void Except_raise(const T *e, const char *file, int line) {
  Except_Frame *p = Except_stack;
  // assert(e);
  if (p == NULL) {
    fprintf(stderr, "Uncaught exception");
    if (e->reason)
      fprintf(stderr, " %s", e->reason);
    else
      fprintf(stderr, " at 0x%p", e);
    if (file && line > 0)
      fprintf(stderr, " raised at %s:%d\n", file, line);
    fprintf(stderr, "aborting...\n");
    fflush(stderr);
    abort();
  }
  p->exception = e;
  p->file = file;
  p->line = line;

  Except_stack = Except_stack->prev;

  longjmp(p->env, Except_raised);
}

#undef T
