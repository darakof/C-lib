#include "dstring.h"
#include "_error.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline int dstrHdrSize(char type) {
  switch (type&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5:
      return sizeof(struct dstr_5);
    case DSTR_TYPE_8:
      return sizeof(struct dstr_8);
    case DSTR_TYPE_16:
      return sizeof(struct dstr_16);
    case DSTR_TYPE_32:
      return sizeof(struct dstr_32);
    case DSTR_TYPE_64:
      return sizeof(struct dstr_64);
  }
  return 0;
}

static inline char dstrReqType(size_t string_size) {
  if (string_size < 1<<5)
    return DSTR_TYPE_5;
  if (string_size < 1<<8)
    return DSTR_TYPE_8;
  if (string_size < 1<<16)
    return DSTR_TYPE_16;
#if (LONG_MAX == LLONG_MAX)
  if (string_size < 1ll<<32)
    return DSTR_TYPE_32;
  return DSTR_TYPE_64;
#else
  return DSTR_TYPE_32;
#endif
}

str dstrnewlen(const char *init, size_t initlen) {
  void *sh;
  str s;
  char type = dstrReqType(initlen);
  // idk the guy said that you shouldnt use type 5 since its bad at appending and thats the reason for empty strings
  if (type == DSTR_TYPE_5 && initlen == 0) type = DSTR_TYPE_8;
  int hdrlen = dstrHdrSize(type);
  unsigned char *fp; // flag pointer

  sh = malloc(hdrlen+initlen+1);
  if (sh == NULL) return NULL;
  if (!init)
    memset(sh, 0, hdrlen+initlen+1);
  s = (char*)sh+hdrlen;
  fp = ((unsigned char*)s)-1;
  switch (type) {
    case DSTR_TYPE_5:
      *fp = type | (initlen << DSTR_TYPE_BITS);
      break;
    case DSTR_TYPE_8: {
      DSTR_TYPE_VAR(8, s);
      sh->len = initlen;
      sh->alloc = initlen;
      *fp = type;
      break;
    }
    case DSTR_TYPE_16: {
      DSTR_TYPE_VAR(16, s);
      sh->len = initlen;
      sh->alloc = initlen;
      *fp = type;
      break;
    }
    case DSTR_TYPE_32: {
      DSTR_TYPE_VAR(32, s);
      sh->len = initlen;
      sh->alloc = initlen;
      *fp = type;
      break;
    }
    case DSTR_TYPE_64: {
      DSTR_TYPE_VAR(64, s);
      sh->len = initlen;
      sh->alloc = initlen;
      *fp = type;
      break;
    }
  }
  if (initlen && init)
    memcpy(s, init, initlen);
  s[initlen] = '\0';
  return s;
}

str dstrempty(void) {
  return dstrnewlen("", 0);
}

str dstrnew(const char *init) {
  size_t initlen = (init == NULL) ? 0 : strlen(init);
  return dstrnewlen(init, initlen);
}

// duplicate
str dstrdup(const str s) {
  return dstrnewlen(s, dstrlen(s));
}

void dstrfree(str s) {
  if (s == NULL) return;
  free((char*)s-dstrHdrSize(s[-1]));
}

void dstrupdatelen(str s) {
  size_t reallen = strlen(s);
  dstrsetlen(s, reallen);
}

void dstrclear(str s) {
  dstrsetlen(s, 0);
  s[0] = '\0';
}

str dstrMakeRoomFor(str s, size_t addlen) {
  void *sh, *newsh;
  size_t avail = dstravail(s);
  size_t len, newlen, reqlen;
  char type, oldtype = s[-1] & DSTR_TYPE_MASK;
  int hdrlen;

  if (avail >= addlen) return s;

  len = dstrlen(s);
  sh = (char*)s-dstrHdrSize(oldtype);
  reqlen = newlen = (len+addlen);
  if (newlen < DSTR_MAX_PREALLOC)
    newlen *= 2;
  else
    newlen += DSTR_MAX_PREALLOC;

  type = dstrReqType(newlen);

  // type 5 does not have a way to store allocated space so dstrMakeRoomFor() must be called every time you append
  if (type == DSTR_TYPE_5) type = DSTR_TYPE_8;

  hdrlen = dstrHdrSize(type);
  assert(hdrlen + newlen + 1 > reqlen); // if size_t overflows
  if (oldtype==type) {
    newsh = realloc(sh, hdrlen+newlen+1);
    if (newsh == NULL) return NULL;
    s = (char*)newsh+hdrlen;
  } else {
    newsh = malloc(hdrlen+newlen+1);
    if (newsh == NULL) return NULL;
    memcpy((char*)newsh+hdrlen, s, len+1);
    free(sh);
    s = (char*)newsh+hdrlen;
    s[-1] = type;
    dstrsetlen(s, len);
  }
  dstrsetalloc(s, newlen);
  return s;
}

str dstrRemoveFreeSpace(str s){
  void *sh, *newsh;
  char type, oldtype = s[-1] & DSTR_TYPE_MASK;
  int hdrlen, oldhdrlen = dstrHdrSize(oldtype);
  size_t len = dstrlen(s);
  size_t avail = dstravail(s);
  sh = (char*)s-oldhdrlen;

  if (avail == 0) return s;

  type = dstrReqType(len);
  hdrlen = dstrHdrSize(type);

  if (oldtype == type || type > DSTR_TYPE_8) {
    newsh = realloc(sh, oldhdrlen+len+1);
    if (newsh == NULL) return NULL;
    s = (char*)newsh+oldhdrlen;
  } else {
    newsh = malloc(hdrlen+len+1);
    if (newsh == NULL) return NULL;
    memcpy((char*)newsh+hdrlen, s, len+1);
    free(sh);
    s = (char*)newsh+hdrlen;
    s[-1] = type;
    dstrsetlen(s, len);
  }
  dstrsetalloc(s, len);
  return s;
}

size_t dstrAllocSize(str s) {
  size_t alloc = dstralloc(s);
  return dstrHdrSize(s[-1])+alloc+1;
}

void *dstrAllocPtr(str s) {
  return (void*) (s-dstrHdrSize(s[-1]));
}

void dstrIncrLen(str s, ssize_t incr) {
  unsigned char flags = s[-1];
  size_t len;
  switch (flags&DSTR_TYPE_MASK) {
    case DSTR_TYPE_5:
      unsigned char *fp = ((unsigned char*)s)-1;
      unsigned char oldlen = DSTR_TYPE_5_LEN(flags);
      assert((incr > 0 && oldlen+incr < 32) || (incr < 0 && oldlen >= (unsigned int)(-incr)));
      *fp = DSTR_TYPE_5 | ((oldlen+incr) << DSTR_TYPE_BITS);
      len = oldlen+incr;
      break;
    case DSTR_TYPE_8: {
      DSTR_TYPE_VAR(8, s);
      assert((incr >= 0 && sh->alloc-sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
      len = (sh->len += incr);
      break;
    }
    case DSTR_TYPE_16: {
      DSTR_TYPE_VAR(16, s);
      assert((incr >= 0 && sh->alloc-sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
      len = (sh->len += incr);
      break;
    }
    case DSTR_TYPE_32: {
      DSTR_TYPE_VAR(32, s);
      assert((incr >= 0 && sh->alloc-sh->len >= (unsigned int)incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
      len = (sh->len += incr);
      break;
    }
    case DSTR_TYPE_64: {
      DSTR_TYPE_VAR(64, s);
      assert((incr >= 0 && sh->alloc-sh->len >= (uint64_t)incr) || (incr < 0 && sh->len >= (uint64_t)(-incr)));
      len = (sh->len += incr);
      break;
    }
    default: len = 0;
  }
  s[len] = '\0';
}

str dstrgrowzero(str s, size_t len) {
  size_t curlen = dstrlen(s);
  if (len <= curlen) return s;
  s = dstrMakeRoomFor(s, len-curlen);
  if (s == NULL) return NULL;

  memset(s+curlen,0,(len-curlen+1));
  dstrsetlen(s, len);
  return s;
}

str dstrcatlen(str s, const void *t, size_t len) {
  size_t curlen = dstrlen(s);

  s = dstrMakeRoomFor(s, len);
  if (s == NULL) return NULL;
  memcpy(s+curlen, t, len);
  dstrsetlen(s, curlen+len);
  s[curlen+len] = '\0';
  return s;
}

str dstrcat(str s, const char *t) {
  return dstrcatlen(s, t, strlen(t));
}

str dstrcatstr(str s, const str t) {
  return dstrcatlen(s, t, dstrlen(t));
}

str dstrcpylen(str s, const char *t, size_t len) {
  if (dstralloc(s) < len) {
    dstrMakeRoomFor(s, len-dstrlen(s));
    if (s == NULL) return NULL;
  }
  memcpy(s, t, len);
  s[len] = '\0';
  dstrsetlen(s, len);
  return s;
}

str dstrcpy(str s, const char *t) {
  return dstrcpylen(s, t, strlen(t));
}

#define DSTR_LLSTR_SIZE 21
int dstrll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    if (value < 0) {
        /* Since v is unsigned, if value==LLONG_MIN then
         * -LLONG_MIN will overflow. */
        if (value != LLONG_MIN) {
            v = -value;
        } else {
            v = ((unsigned long long)LLONG_MAX) + 1;
        }
    } else {
        v = value;
    }

    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sdsll2str(), but for unsigned long long type. */
int dstrull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

str dstrfromlonglong(long long value) {
  char buf[DSTR_LLSTR_SIZE];
  int len = dstrll2str(buf, value);

  return dstrnewlen(buf, len);
}

str dstrcatvprintf(str s, const char *fmt, va_list ap) {
  va_list cpy;
  char staticbuf[1024], *buf = staticbuf, *t;
  size_t buflen = strlen(fmt)*2;
  int bufstrlen;

  if (buflen > sizeof(staticbuf)) {
    buf = malloc(buflen);
    if (buf == NULL) return NULL;
  } else {
    buflen = sizeof(staticbuf);
  }

  while (1) {
    va_copy(cpy,ap);
    bufstrlen = vsnprintf(buf, buflen, fmt, cpy);
    va_end(cpy);
    if (bufstrlen < 0) {
      if (buf != staticbuf) free(buf);
      return NULL;
    }
    if (((size_t)bufstrlen) >= buflen) {
      if (buf != staticbuf) free(buf);
      buflen = ((size_t)bufstrlen) + 1;
      buf = malloc(buflen);
      if (buf == NULL) return NULL;
      continue;
    }
    break;
  }

  t = dstrcatlen(s, buf, bufstrlen);
  if (buf != staticbuf) free(buf);
  return t;
}

str dstrcatprintf(str s, const char *fmt, ...) {
  va_list ap;
  char *t;
  va_start(ap, fmt);
  t = dstrcatvprintf(s, fmt, ap);
  va_end(ap);
  return t;
}

/* This function is similar to sdscatprintf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the sds string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %S - SDS string
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
str dstrcatfmt(str s, char const *fmt, ...) {
    size_t initlen = dstrlen(s);
    const char *f = fmt;
    long i;
    va_list ap;

    /* To avoid continuous reallocations, let's start with a buffer that
     * can hold at least two times the format string itself. It's not the
     * best heuristic but seems to work in practice. */
    s = dstrMakeRoomFor(s, initlen + strlen(fmt)*2);
    va_start(ap,fmt);
    f = fmt;    /* Next format specifier byte to process. */
    i = initlen; /* Position of the next byte to write to dest str. */
    while(*f) {
        char next, *str;
        size_t l;
        long long num;
        unsigned long long unum;

        /* Make sure there is always space for at least 1 char. */
        if (dstravail(s)==0) {
            s = dstrMakeRoomFor(s,1);
        }

        switch(*f) {
        case '%':
            next = *(f+1);
            if (next == '\0') break;
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap,char*);
                l = (next == 's') ? strlen(str) : dstrlen(str);
                if (dstravail(s) < l) {
                    s = dstrMakeRoomFor(s,l);
                }
                memcpy(s+i,str,l);
                dstrinclen(s,l);
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap,int);
                else
                    num = va_arg(ap,long long);
                {
                    char buf[DSTR_LLSTR_SIZE];
                    l = dstrll2str(buf,num);
                    if (dstravail(s) < l) {
                        s = dstrMakeRoomFor(s,l);
                    }
                    memcpy(s+i,buf,l);
                    dstrinclen(s,l);
                    i += l;
                }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap,unsigned int);
                else
                    unum = va_arg(ap,unsigned long long);
                {
                    char buf[DSTR_LLSTR_SIZE];
                    l = dstrull2str(buf,unum);
                    if (dstravail(s) < l) {
                        s = dstrMakeRoomFor(s,l);
                    }
                    memcpy(s+i,buf,l);
                    dstrinclen(s,l);
                    i += l;
                }
                break;
            default: /* Handle %% and generally %<unknown>. */
                s[i++] = next;
                dstrinclen(s,1);
                break;
            }
            break;
        default:
            s[i++] = *f;
            dstrinclen(s,1);
            break;
        }
        f++;
    }
    va_end(ap);

    /* Add null-term */
    s[i] = '\0';
    return s;
}

str dstrtrim(str s, const char *cset) {
  char *end, *sp, *ep;
  size_t len;

  sp = s;
  ep = end = s+dstrlen(s)-1;
  while(sp <= end && strchr(cset, *sp)) sp++;
  while(ep > sp && strchr(cset, *ep)) ep--;
  len = (ep-sp)+1;
  if (s != sp) memmove(s, sp, len);
  s[len] = '\0';
  dstrsetlen(s,len);
  return s;
}

void dstrrange(str s, ssize_t start, ssize_t end) {
  size_t newlen, len = dstrlen(s);

  if (len == 0) return;
  if (start < 0) {
      start = len+start;
      if (start < 0) start = 0;
  }
  if (end < 0) {
      end = len+end;
      if (end < 0) end = 0;
  }
  newlen = (start > end) ? 0 : (end-start)+1;
  if (newlen != 0) {
      if (start >= (ssize_t)len) {
          newlen = 0;
      } else if (end >= (ssize_t)len) {
          end = len-1;
          newlen = (end-start)+1;
      }
  }
  if (start && newlen) memmove(s, s+start, newlen);
  s[newlen] = 0;
  dstrsetlen(s,newlen);
}

void dstrtolower(str s) {
  size_t len = dstrlen(s), j;
  for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}

void dstrtoupper(str s) {
  size_t len = dstrlen(s), j;
  for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}

int dstrcmp(const str s1, const str s2) {
  size_t l1, l2, minlen;
  int cmp;

  l1 = dstrlen(s1);
  l2 = dstrlen(s2);
  minlen = (l1 < l2) ? l1 : l2;
  cmp = memcmp(s1,s2,minlen);
  if (cmp == 0) return l1>l2? 1: (l1<l2? -1: 0);
  return cmp;
}

str *dstrsplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count) {
  int elements = 0, slots = 5;
  long start = 0, j;
  str *tokens;

  if (seplen < 1 || len <= 0) {
    *count = 0;
    return NULL;
  }

  tokens = malloc(sizeof(str)*slots);
  if (tokens == NULL) return NULL;

  for (j = 0; j < (len-(seplen-1)); j++) {
    /* make sure there is room for the next element and the final one */
    if (slots < elements+2) {
      str *newtokens;

      slots *= 2;
      newtokens = realloc(tokens,sizeof(str)*slots);
      if (newtokens == NULL) goto cleanup;
      tokens = newtokens;
    }
    /* search the separator */
    if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
      tokens[elements] = dstrnewlen(s+start,j-start);
      if (tokens[elements] == NULL) goto cleanup;
      elements++;
      start = j+seplen;
      j = j+seplen-1; /* skip the separator */
    }
  }
  /* Add the final element. We are sure there is room in the tokens array. */
  tokens[elements] = dstrnewlen(s+start,len-start);
  if (tokens[elements] == NULL) goto cleanup;
  elements++;
  *count = elements;
  return tokens;

cleanup:
  {
      int i;
      for (i = 0; i < elements; i++) dstrfree(tokens[i]);
      free(tokens);
      *count = 0;
      return NULL;
  }
}

void dstrfreesplitres(str *tokens, int count) {
  if (!tokens) return;
  while (count--)
    dstrfree(tokens[count]);
  free(tokens);
}

str dstrcatrepr(str s, const char *p, size_t len) {
  s = dstrcatlen(s,"\"",1);
  while(len--) {
    switch(*p) {
    case '\\':
    case '"':
      s = dstrcatprintf(s,"\\%c",*p);
      break;
    case '\n': s = dstrcatlen(s,"\\n",2); break;
    case '\r': s = dstrcatlen(s,"\\r",2); break;
    case '\t': s = dstrcatlen(s,"\\t",2); break;
    case '\a': s = dstrcatlen(s,"\\a",2); break;
    case '\b': s = dstrcatlen(s,"\\b",2); break;
    default:
      if (isprint(*p))
        s = dstrcatprintf(s,"%c",*p);
      else
        s = dstrcatprintf(s,"\\x%02x",(unsigned char)*p);
      break;
    }
    p++;
  }
  return dstrcatlen(s,"\"",1);
}

/* Helper function for sdssplitargs() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for sdssplitargs() that converts a hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

str *dstrsplitargs(const char *line, int *argc) {
  
}

str dstrmapchars(str s, const char *from, const char *to, size_t setlen);
str dstrjoin(char **argv, int argc, char *sep);
str dstrjoindstr(str *argv, int argc, const char *sep, size_t seplen);


void *dstr_malloc(size_t size);
void *dstr_realloc(void *ptr, size_t size);
void dstr_free(void *ptr);


// int main(void) {
//   struct dstr_5 *str = malloc(sizeof(struct dstr_5));
//   str->flag = 0xF0;
//   char *s = (char*)(str+sizeof(struct dstr_5));
//   printf("len: %zu\n", dstrlen(s));
// }
