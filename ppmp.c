/* -*- C -*- 
 Copyright (c) 2017 Contact Software.

 All rights reserved. This program and the accompanying materials are
 made available under the terms of the Eclipse Public License v1.0.

 The Eclipse Public License is available at 
     http://www.eclipse.org/legal/epl-v10.html
*/

#include "ppmp.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static const char *CONTENT_SPEC_PREFIX = "urn:spec://eclipse.org/unide/";

static const char *CONTENT_SPEC[] = {
  "measurement-message#v2",
  "machine-message#v2",
  "process-message#v2"
};

int fwrite_wrap(void *user_data, const char *buf, int len) {
  return (int) fwrite(buf, len, 1, (FILE*) user_data);
}

int ppmp_init(PPMP *ppmp, void *user_data, ppmp_write_func f_write, int flags) {
  ppmp->user_data = user_data;
  ppmp->join = 0;
  ppmp->flags = flags;
  if (f_write == 0) {
    ppmp->user_data = (void*) stdout;
    f_write = fwrite_wrap;
  }
  ppmp->f_write = f_write;
  ppmp->stackp = 0;
  return 0;
}

int ppmp_push(PPMP *ppmp, char c) {
  if (ppmp->stackp < PPMP_STACK_SIZE) {
    ppmp->stack[ppmp->stackp++] = c;
    return 0;
  }
  return -1;
}

char ppmp_pop(PPMP *ppmp) {
  if (ppmp->stackp > 0) {
    return ppmp->stack[--ppmp->stackp];
  }
  return 0;
}

int ppmp_writez(PPMP *ppmp, const char *buf) {
  return ppmp->f_write(ppmp->user_data, buf, strlen(buf));
}

int ppmp_writen(PPMP *ppmp, ...) {
  va_list vargs;
  const char *buf;
  va_start(vargs, ppmp);
  buf = va_arg(vargs, const char*);
  while (buf) {
    ppmp_writez(ppmp, buf);
    buf = va_arg(vargs, const char*);
  }
  va_end(vargs);
  return 0;
}

int ppmp_string(PPMP *ppmp, const char *str) {
  return ppmp_writen(ppmp, "\"", str, "\"", 0);
}

int ppmp_join(PPMP *ppmp) {
  char buf[3] = "  ";
  if (ppmp->join) {
    buf[0] = ppmp->join;
    ppmp_writez(ppmp, buf);
  }
  return 0;
}

int ppmp_property(PPMP *ppmp, const char *name) {
  ppmp_join(ppmp);
  if (ppmp->flags & PPMP_PRETTY) {
    int i;
    ppmp_writez(ppmp, "\n");
    for (i = 0; i < ppmp->stackp; i++) {
      ppmp_writez(ppmp, "  ");
    }
  }
  ppmp_string(ppmp, name);
  ppmp_writez(ppmp, ": ");
  ppmp->join = ',';
  return 0;
}

int ppmp_object(PPMP *ppmp, ...) {
  ppmp->join = 0;
  ppmp_writez(ppmp, "{");
  ppmp_push(ppmp, '}');
  return 0;
}

int ppmp_list(PPMP *ppmp, ...) {
  ppmp->join = 0;
  ppmp_writez(ppmp, "[");
  ppmp_push(ppmp, ']');
  return 0;
}

int ppmp_end(PPMP *ppmp) {
  char buf[2] = " ";
  char c = ppmp_pop(ppmp);
  int i;
  if (ppmp->flags & PPMP_PRETTY) {
    ppmp_writez(ppmp, "\n");
    for (i = 0; i < ppmp->stackp; i++) {
      ppmp_writez(ppmp, "  ");
    }
  }
  buf[0] = c;
  ppmp_writez(ppmp, buf);
  ppmp->join = ',';
  return 0;
}

int ppmp_measurement_payload(PPMP *ppmp) {
  ppmp_object(ppmp);
  ppmp_property(ppmp, "content-spec");
  ppmp_string(ppmp, "urn:spec://eclipse.org/unide/measurement-message#v2");
  return 0;
}


#define EMIT_PROPERTY(PPMP, NAME)		\
  if (NAME) {					\
     ppmp_property(PPMP, #NAME);		\
     ppmp_string(PPMP, NAME);			\
  }


int ppmp_vmeta(PPMP *ppmp, va_list args) {
  const char *key, *value;
  key = va_arg(args, const char*);
  if (key) {
    ppmp_property(ppmp, "metaData");
    ppmp_object(ppmp);
    while (key) {
      value = va_arg(args, const char*);
      ppmp_property(ppmp, key);
      ppmp_string(ppmp, value);
      key = va_arg(args, const char*);
    }
    ppmp_end(ppmp);
  }
  return 0;
}

int ppmp_device(PPMP *ppmp, const char *deviceID,
		const char *operationalStatus, ...) {
  va_list args;
  va_start(args, operationalStatus);
  ppmp_property(ppmp, "device");
  ppmp_object(ppmp);
    EMIT_PROPERTY(ppmp, deviceID);
    EMIT_PROPERTY(ppmp, operationalStatus);
    ppmp_vmeta(ppmp, args);
  ppmp_end(ppmp);
  va_end(args);
  return 0;
}

int ppmp_part(PPMP *ppmp,
	      const char *partTypeID,
	      const char *partID,
	      const char *result,
	      const char *code,
	      ...) {
  va_list args;
  va_start(args, code);
  ppmp_property(ppmp, "part");
  ppmp_object(ppmp);
    EMIT_PROPERTY(ppmp, partTypeID);
    EMIT_PROPERTY(ppmp, partID);
    EMIT_PROPERTY(ppmp, result);
    EMIT_PROPERTY(ppmp, code);
    ppmp_vmeta(ppmp, args);
  ppmp_end(ppmp);
  va_end(args);
  return 0;
}

int ppmp_measurements(PPMP *ppmp) {
  ppmp_property(ppmp, "measurements");
  ppmp_list(ppmp);
  return 0;
}

int ppmp_measurement(PPMP *ppmp,
		     const char *ts,
		     const char *result,
		     const char *code) {
  ppmp_object(ppmp);
  EMIT_PROPERTY(ppmp, ts);
  EMIT_PROPERTY(ppmp, result);
  EMIT_PROPERTY(ppmp, code);
  ppmp_property(ppmp, "series");
  ppmp_object(ppmp);
  return 0;
}

int ppmp_offsets(PPMP *ppmp, int count, int *offsets) {
  int i;
  char buf[20];
  ppmp_property(ppmp, "$_time");
  ppmp_list(ppmp);
  for (i = 0; i < count; i++) {
    if (i==0) {
      sprintf(buf, "%d", offsets[i]);
    } else {
      sprintf(buf, ", %d", offsets[i]);
    }
    ppmp_writez(ppmp, buf);
  }
  ppmp_end(ppmp);
  return 0;
}

int ppmp_samples(PPMP *ppmp, const char *name, int count, double *offsets) {
  int i;
  char buf[20];
  ppmp_property(ppmp, name);
  ppmp_list(ppmp);
  for (i = 0; i < count; i++) {
    if (i==0) {
      sprintf(buf, "%g", offsets[i]);
    } else {
      sprintf(buf, ", %g", offsets[i]);
    }
    ppmp_writez(ppmp, buf);
  }
  ppmp_end(ppmp);
  return 0;
}

int ppmp_finish(PPMP *ppmp) {
  while (ppmp->stackp) {
    ppmp_end(ppmp);
  }
  return 0;
}
