#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
  char *description;
  char *function;
  char *args;
} Error;

void (*error_callback)(Error *error);

void inline error_set_callback(void (*error_callback_fn_ptr)(Error *error)) {
  error_callback = error_callback_fn_ptr;
}

Error inline *Create_Error(char *desc, char *func, char *_args) {
  Error * err = malloc(sizeof(Error));
  err->description = desc;
  err->function = func;
  err->args = _args;
  return err;
}

#endif // ERROR_H
