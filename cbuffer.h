/* -*- mode: C -*-
 * $Id$
 *
 * Copyright (c) 1990 - 2015 by CONTACT Software GmbH.
 * All rights reserved.
 * http://www.contact.de/
 */

#ifndef CBUFFER_H
#define CBUFFER_H

#pragma once

#include <stdarg.h>
#include <string.h>

/** cbuffer provides a dynamically growing buffer object. The buffer
    can be manipulated with printf-style calls to insert or append
    data. cbuffer objects can also hold binary data (through
    cbuffer_nappend). 

    cbuffer_allocator objects encapsulate a memory management strategy
    for cbuffer objects. The standard strategy is to use
    free/malloc/realloc from the standard library.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cbuffer_allocator {
  void (*pfree)(void*,struct _cbuffer_allocator*);
  void *(*pmalloc)(size_t,struct _cbuffer_allocator*);
  void *(*prealloc)(void*,size_t,struct _cbuffer_allocator*);
  void *pdata;
} cbuffer_allocator;

typedef struct _cbuffer {
  char *buf;             /* memory location of buffer data */
  char *mark;            /* current write mark */
  size_t bufsiz;         /* bytes available at buf */
  cbuffer_allocator *mm; /* the allocator associated with this cbuffer */
} cbuffer;

  /** Create a cbuffer object, without allocating memory for it. */
  void cbuffer_create(cbuffer*);
  /** Create a cbuffer with allocating memory. size may be 0 in which
      case some reasonable default is chosen. Caller can also pass an
      allocator object. If allocator is 0, standard memory management
      is used. */
  /*void cbuffer_create2(cbuffer*, size_t size, cbuffer_allocator *allocator); */
  /** Allocate memory to a cbuffer.  size may be 0 in which case some
      reasonable default is chosen. If the cbuffer already is
      allocated, the memory is discarded. */
  void cbuffer_alloc(cbuffer*, size_t size);
  /** Reset cbuffer to contain nothing. */
  void cbuffer_clear(cbuffer*);
  /** Dispose all allocated memory. */
  void cbuffer_dispose(cbuffer*);
  /** Copy the content of the second cbuffer into the first. */
  void cbuffer_copy(cbuffer*, const cbuffer*);
  /** Make sure the cbuffer has at least new_size bytes allocated (may
      be more). */
  void cbuffer_grow(cbuffer*, size_t new_size);
  /** Make sure that one can safely copy bytes_needed into the
      location pointed to by *p (which is somewhere inside the
      allocated memory of the cbuffer) without overwriting buffer
      boundaries. *p is passed by-reference, because it may be
      corrected when the buffer needs to be reallocated. */
  void cbuffer_check(cbuffer*, char **p, size_t bytes_needed);
  /** vprintf-style like formatting. */
  void cbuffer_vformat(cbuffer*, const char *format, va_list args);
  /** printf-style like formatting. */
  void cbuffer_format(cbuffer*, const char *format, ...);
  /** Append text to the buffer. */
  void cbuffer_append(cbuffer*, const char *text);
  /** Append (may-be-binary) size bytes starting at text to the buffer. */
  void cbuffer_nappend(cbuffer*, const char *text, size_t size);
  /** printf-style like append text to the buffer. */
  void cbuffer_appendf(cbuffer*, const char *format, ...);
  /** vprintf-style like append text to the buffer. */
  void cbuffer_vappendf(cbuffer*, const char *format, va_list args);
  /** Return != 0 iff the buffer is empty. */
  int cbuffer_empty(const cbuffer*);
  /* Liefert die Anzahl an Zeichen die noch reinpassen, ohne das ein grow
     passieren muss */
  size_t cbuffer_space_left(const cbuffer*);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif 
