#include "ppmp.h"
#include "cbuffer.h"

#include <stdio.h>
#include <string.h>

/* The PPMP library does not allocate memory for JSON payloads by
   itself. Instead, a PPMP object has to be initialized with a 'write'
   function. In this example, we use a simple buffer data
   structure, and pass the 'write_buffer' to 'ppmp_init'. */
int write_buffer(void *user_data, const char *data, int len) {
  cbuffer *buf = (cbuffer*) user_data;
  cbuffer_nappend(buf, data, len);
  return 0;
}


int main(int argc, const char *argv[]) {
  PPMP ppmpbuf;
  PPMP *ppmp = &ppmpbuf;
  cbuffer _buf;
  cbuffer *buf = &_buf;
  cbuffer_create(buf);
  cbuffer_alloc(buf, 1000);
  ppmp_init(ppmp, buf, write_buffer, PPMP_PRETTY);
  ppmp_measurement_payload(ppmp);
  ppmp_device(ppmp, "My-Device", "running",
	      "Manufacture", "Bosch",
	      NULL);
  ppmp_finish(ppmp);

  fputs(buf->buf, stdout);
  return 0;
}
