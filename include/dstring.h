#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#define DSTR_MAX_PREALLOC (1024*1024)

#include "error.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

// we use char* because we can store anything in such data type as well as a
// string (like we wanted)
typedef char *str;

struct __attribute__((__packed__)) dstr_5 {
  unsigned char flag; // 5 higher bits for length, 3 lower bits for type
  char data[]; // flexiable array member to store everything as one sequence
};

struct __attribute__((__packed__)) dstr_8 {
  uint8_t len;
  uint8_t alloc;
  unsigned char flag;
  char data[];
};

struct __attribute__((__packed__)) dstr_16 {
  uint16_t len;
  uint16_t alloc;
  unsigned char flag;
  char data[];
};

struct __attribute__((__packed__)) dstr_32 {
  uint32_t len;
  uint32_t alloc;
  unsigned char flag;
  char data[];
};

struct __attribute__((__packed__)) dstr_64 {
  uint64_t len;
  uint64_t alloc;
  unsigned char flag;
  char data[];
};

// string types and mask
#define DSTR_TYPE_5 0
#define DSTR_TYPE_8 1
#define DSTR_TYPE_16 2
#define DSTR_TYPE_32 3
#define DSTR_TYPE_64 4
#define DSTR_TYPE_MASK 7
#define DSTR_TYPE_BITS 3

// helpers
#define DSTR_TYPE_VAR(T, s) struct dstr_##T *sh = (void*)((s) - sizeof(struct dstr_##T)) // get the start of the type struct and create the variable
#define DSTR_TYPE(T, s) ((struct dstr_##T *)((s)-(sizeof(struct dstr_##T)))) // get the struct type without creating a var
#define DSTR_TYPE_5_LEN(f) ((f) >> DSTR_TYPE_BITS) // get the length from the 5th type

static inline size_t dstrlen(const str s) {
  unsigned char flag = s[-1];
  switch (flag & DSTR_TYPE_MASK) {
  case DSTR_TYPE_5:
    return DSTR_TYPE_5_LEN(flag);
  case DSTR_TYPE_8:
    return DSTR_TYPE(8, s)->len;
  case DSTR_TYPE_16:
    return DSTR_TYPE(16, s)->len;
  case DSTR_TYPE_32:
    return DSTR_TYPE(32, s)->len;
  case DSTR_TYPE_64:
    return DSTR_TYPE(64, s)->len;
  }

  return 0;
}

static inline size_t dstravail(const str s) {
  unsigned char flag = s[-1];
  switch (flag & DSTR_TYPE_MASK) {
    case DSTR_TYPE_5: {
      return 0;
    }
    case DSTR_TYPE_8: {
      DSTR_TYPE_VAR(8, s);
      return sh->alloc - sh->len;
    }
    case DSTR_TYPE_16: {
      DSTR_TYPE_VAR(16, s);
      return sh->alloc - sh->len;
    }
    case DSTR_TYPE_32: {
      DSTR_TYPE_VAR(32, s);
      return sh->alloc - sh->len;
    }
    case DSTR_TYPE_64: {
      DSTR_TYPE_VAR(64, s);
      return sh->alloc - sh->len;
    }
  }

  return 0;
}

static inline void dstrsetlen(str s, size_t newlen) {
  unsigned char flag = s[-1];
  switch (flag&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5 : {
      unsigned char *fp = ((unsigned char *)s)-1;
      *fp = DSTR_TYPE_5 | (newlen<<DSTR_TYPE_BITS);
    }
    case DSTR_TYPE_8:
      DSTR_TYPE(8,s)->len = newlen;
      break;
    case DSTR_TYPE_16:
      DSTR_TYPE(16,s)->len = newlen;
      break;
    case DSTR_TYPE_32:
      DSTR_TYPE(32,s)->len = newlen;
      break;
    case DSTR_TYPE_64:
      DSTR_TYPE(64,s)->len = newlen;
      break;
  }
}

static inline void dstrinclen(str s, size_t inc) {
  unsigned char flag = s[-1];
  switch (flag&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5 : {
      unsigned char *fp = ((unsigned char *)s)-1;
      unsigned char newlen = DSTR_TYPE_5_LEN(flag)+inc;
      *fp = DSTR_TYPE_5 | (newlen<<DSTR_TYPE_BITS);
    }
    case DSTR_TYPE_8:
      DSTR_TYPE(8,s)->len += inc;
      break;
    case DSTR_TYPE_16:
      DSTR_TYPE(16,s)->len += inc;
      break;
    case DSTR_TYPE_32:
      DSTR_TYPE(32,s)->len += inc;
      break;
    case DSTR_TYPE_64:
      DSTR_TYPE(64,s)->len += inc;
      break;
  }
}

static inline size_t dstralloc(const str s) {
  unsigned char flag = s[-1];
  switch (flag&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5:
      return DSTR_TYPE_5_LEN(flag);
    case DSTR_TYPE_8:
      return DSTR_TYPE(8,s)->alloc;
    case DSTR_TYPE_16:
      return DSTR_TYPE(16,s)->alloc;
    case DSTR_TYPE_32:
      return DSTR_TYPE(32,s)->alloc;
    case DSTR_TYPE_64:
      return DSTR_TYPE(64,s)->alloc;
  }
  return 0;
}

static inline void dstrsetalloc(str s, size_t newlen) {
  unsigned char flag = s[-1];
  switch (flag&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5 :
      // we dont have an alloc in type 5
      break;
    case DSTR_TYPE_8:
      DSTR_TYPE(8,s)->alloc = newlen;
      break;
    case DSTR_TYPE_16:
      DSTR_TYPE(16,s)->alloc = newlen;
      break;
    case DSTR_TYPE_32:
      DSTR_TYPE(32,s)->alloc = newlen;
      break;
    case DSTR_TYPE_64:
      DSTR_TYPE(64,s)->alloc = newlen;
      break;
  }
}

str dstrnewlen(const char *init, size_t initlen);
str dstrnew(const char *init);
str dstrempty(void);
str dstrdup(const str s);
void dstrfree(str s);
str dstrgrowzero(str s, size_t len);
str dstrcatlen(str s, const void *t, size_t len);
str dstrcat(str s, const char *t);
str dstrcatstr(str s, const str t);
str dstrcpylen(str s, const char *t, size_t len);
str dstrcpy(str s, const char *t);

str dstrcatvprintf(str s, const char *fmt, va_list ap);
#ifdef __GNUC__
str dstrcatprintf(str s, const char *fmt, ...)
  __attribute__((format(printf, 2, 3)));
#else
str dstrcatprintf(str s, const chat *fmt, ...);
#endif

str dstrcatfmt(str s, char const *fmt, ...);
str dstrtrim(str s, const char *cset);
void dstrrange(str s, ssize_t start, ssize_t end);
void dstrupdatelen(str s);
void dstrclear(str s);
int dstrcmp(const str s1, const str s2);
str *dstrsplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);
void dstrfreesplitres(str *tokens, int count);
void dstrtolower(str s);
void dstrtoupper(str s);
str dstrfromlonglong(long long value);
str dstrcatrepr(str s, const char *p, size_t len);
str *dstrsplitargs(const char *line, int *argc);
str dstrmapchars(str s, const char *from, const char *to, size_t setlen);
str dstrjoin(char **argv, int argc, char *sep);
str dstrjoindstr(str *argv, int argc, const char *sep, size_t seplen);

str dstrMakeRoomFor(str s, size_t addlen);
void dstrIncrLen(str s, ssize_t incr);
str dstrRemoveFreeSpace(str s);
size_t dstrAllocSize(str s);
void *dstrAllocPtr(str s);

void *dstr_malloc(size_t size);
void *dstr_realloc(void *ptr, size_t size);
void dstr_free(void *ptr);

#endif // DYNAMIC_STRING_H
