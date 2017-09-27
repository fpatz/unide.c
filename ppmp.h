/* -*- C -*- 
 Copyright (c) 2017 Contact Software.

 All rights reserved. This program and the accompanying materials are
 made available under the terms of the Eclipse Public License v1.0.

 The Eclipse Public License is available at 
     http://www.eclipse.org/legal/epl-v10.html
*/

#ifndef PPMP_H
#define PPMP_H

#define PPMP_PRETTY 1 /* Output indented JSON for the PPMP message. */

#define PPMP_MEASUREMENT 0
#define PPMP_MESSAGE 1
#define PPMP_PROCESS 2

/* stdlib gives us NULL */
#include <stdlib.h>

/* ... */
typedef int (*ppmp_write_func)(void *user_data, const char *buf, int len);

/* The `PPMP` struct holds the context for the API and is passed to
   all functions. */
typedef struct {
#define PPMP_STACK_SIZE 20
  void *user_data;
  ppmp_write_func f_write;
  unsigned short flags;
  char join;
  short stackp;
  char stack[PPMP_STACK_SIZE];
} PPMP;

/* Initialize a PPMP context. Return 0 on success, an error code
   otherwise. */
int ppmp_init(PPMP *ppmp, void *user_data, ppmp_write_func f_write, int flags);

/* ... */
int ppmp_measurement_payload(PPMP *ppmp);

/* device -- Contains information about the device; required: yes

   `ppmp_device` must be called after `ppmp_measurement_payload` to
   form a correct message.
 */
		/* The unique ID of the device. As this is used to
		   identify a device independently from time or
		   location. The ID itself must be stable and
		   unique. The recommendation is to use a universally
		   unique identifier (UUID).

		   Required: true
		   Size restriction: 36
		   Note: reprentation could follow GIAI, UUID or others
		   Example:
		   ...
		   "deviceID": "6e7807d0-5491-11e6-9d32-02423234b390"
		   ...
		*/
		/* The operationalStatus describes the status or mode
		   of a device.

		   Required: no (may be NULL)
		   Size restriction: no */

int ppmp_device(PPMP *ppmp,
		const char *deviceID,
		const char *operationalStatus,
		/* Variable arguments for meta data, terminated by
		   NULL */
		...);

/* ... */
int ppmp_part(PPMP *ppmp,
	      const char *partTypeID,
	      const char *partID,
	      const char *result,
	      const char *code,
	      ...   /* Variable arguments for meta data, terminated by
		 NULL */
	      );


int ppmp_measurements(PPMP *ppmp);
int ppmp_measurement(PPMP *ppmp,
		     const char *ts,
		     const char *result,
		     const char *code);

int ppmp_offsets(PPMP *ppmp, int count, int *offsets);
int ppmp_samples(PPMP *ppmp, const char *name, int count, double *offsets);
int ppmp_finish(PPMP *ppmp);
int ppmp_end(PPMP *ppmp);

#endif /* PPMP_H */
