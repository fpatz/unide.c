#include "ppmp.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  char *buf;
  char *cursor;
} StaticBuffer;

/* The PPMP library does not allocate memory for JSON payloads by
   itself. Instead, a PPMP object has to be initialized with a 'write'
   function. In this example, we use a simple buffer data
   structure, and pass the 'write_buffer' to 'ppmp_init'. */
int write_buffer(void *user_data, const char *data, int len) {
  StaticBuffer *sb = (StaticBuffer*) user_data;
  memcpy(sb->cursor, data, len);
  sb->cursor += len;
  *sb->cursor = 0;
  return 0;
}


int main(int argc, const char *argv[]) {
  PPMP ppmp;
  char msgbuf[2048];
  StaticBuffer sb;
  sb.buf = msgbuf;
  sb.cursor = msgbuf;
  ppmp_init(&ppmp, &sb, write_buffer, PPMP_PRETTY);
  ppmp_measurement_payload(&ppmp);
  ppmp_device(&ppmp, "My-Device", "running",
	      "Manufacture", "Bosch",
	      NULL);
  ppmp_finish(&ppmp);

  fputs(sb.buf, stdout);
  return 0;
}
