/* -*- mode: C; coding: us-ascii -*-
 * $Id$
 *
 * Copyright (c) 1990 - 2017 by CONTACT Software GmbH.
 * All rights reserved.
 * https://www.contact-software.com/
 */

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include "cbuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

static void std_free(void *p, cbuffer_allocator *allocator) { free(p); }
static void* std_malloc(size_t n, cbuffer_allocator *allocator) { return malloc(n); }
static void* std_realloc(void *p, size_t n, cbuffer_allocator *allocator) { return realloc(p, n); }

static cbuffer_allocator std_mm = { std_free, std_malloc, std_realloc, 0};

/* */
#define CBUFPAGESIZE 1024

void cbuffer_create(cbuffer *b) {
  b->mark = b->buf = 0;
  b->bufsiz = 0;
  b->mm = &std_mm;
}

void cbuffer_clear(cbuffer *b) {
  b->mark = b->buf;
}

void cbuffer_alloc(cbuffer *b, size_t size) {
  if (b->buf) cbuffer_dispose(b);
  if (size == 0) size = CBUFPAGESIZE/3;
  b->mark = b->buf = (char*) b->mm->pmalloc(size, b->mm);
  b->bufsiz = size;
}

void cbuffer_dispose(cbuffer *b) {
  b->mm->pfree(b->buf, b->mm);
  b->mark = b->buf = 0;
}

void cbuffer_copy(cbuffer *b, const cbuffer *other) {
  if (other != b) {
    cbuffer_dispose(b);
    cbuffer_alloc(b, other->bufsiz);
#ifdef _WIN32
    memcpy_s(b->buf, b->bufsiz, other->buf, other->bufsiz);
#else
    memcpy(b->buf, other->buf, b->bufsiz);
#endif
  }
}

void cbuffer_grow(cbuffer *b, size_t new_size) {
  size_t diff;
  if (0==new_size)
  {
    b->bufsiz *=2;
  }
  else
  {
    while(b->bufsiz < new_size){
      b->bufsiz *= 2;
    }
  }
  diff = b->mark - b->buf;
  b->buf = (char*) b->mm->prealloc(b->buf, b->bufsiz, b->mm);
  b->mark = b->buf + diff;
}

void cbuffer_check(cbuffer *b, char **p, size_t bytes_needed) {
  if (*p + bytes_needed > b->buf + b->bufsiz) {
    cbuffer_grow(b, (*p + bytes_needed) - b->buf);
    *p = b->mark;
  }
}

static void _cbuffer_vformat(cbuffer *b, const char *format, va_list args_in) {
  int ret = 0;
  size_t space_left = cbuffer_space_left(b);
#ifdef _WIN32
  /* use the more secure vsnprintf_s here */
  while (0 > (ret = vsnprintf_s(b->mark, space_left, _TRUNCATE, format, args_in)) || ret >= (int)space_left) {
    cbuffer_grow(b, 0);
    space_left = cbuffer_space_left(b);
  }
#else
  /* We possibly apply vsnprintf multiple times on args. But after calling
     vsnprintf the value of args is undefined (see manpage to vsnprintf:
     "Because they invoke the va_arg macro, the value of ap is undefined after
     the call."), so we need to use a copy of args.
     VC supports va_copy starting with vs2013 :-( */
  va_list args;
  va_copy(args, args_in);
  /* Consider different vsnprintf return values: old implementations
     return -1 if the output does not fit into the buffer, newer (C99)
     implementations return the number of characters that would have
     been written if the buffer had been big enough. Weird. */
  while (0 > (ret = vsnprintf(b->mark, space_left, format, args)) || ret >= (int) space_left) {
    va_end(args);
    va_copy(args, args_in);
    cbuffer_grow(b, 0);
    space_left = cbuffer_space_left(b);
  }
  va_end(args);
#endif
  b->mark += ret;
}

void cbuffer_vformat(cbuffer *b, const char *format, va_list args) {
  b->mark = b->buf;
  _cbuffer_vformat(b, format, args);
}

void cbuffer_format(cbuffer *b, const char *format, ...) {
  va_list args;
  va_start(args, format);
  cbuffer_vformat(b, format, args);
  va_end(args);
}

void cbuffer_vappendf(cbuffer *b, const char *format, va_list args) {
  _cbuffer_vformat(b, format, args);
}

void cbuffer_appendf(cbuffer *b, const char *format, ...) {
  va_list args;
  va_start(args, format);
  cbuffer_vappendf(b, format, args);
  va_end(args);
}

int cbuffer_empty(const cbuffer *b) {
  return b->buf == b->mark;
}

size_t cbuffer_space_left(const cbuffer *b) {
  if (b) {
    return (b->bufsiz) - (b->mark - b->buf);
  } else {
    return 0;
  }
}

void cbuffer_append(cbuffer *b, const char *text) {
  /* Make sure to copy \0. */
  cbuffer_nappend(b, text, strlen(text) + 1);
  /* Step back to \0 to continue appending there. */
  b->mark -= 1;
}

void cbuffer_nappend(cbuffer *b, const char *data, size_t size) {
  cbuffer_check(b, &(b->mark), size);
#ifdef _WIN32
  memcpy_s(b->mark, cbuffer_space_left(b), data, size);
#else
  memcpy(b->mark, data, size);
#endif
  b->mark = b->mark + size;
}
