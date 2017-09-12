#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum PPMP_MESSAGE_TYPE {
  PPMP_MEASUREMENT,
  PPMP_MESSAGE,
  PPMP_PROCESS
};

const char* MEASUREMENT_V2 = "\"urn:spec://eclipse.org/unide/measurement-message#v2\"";

typedef int (*_write_func)(void *ctx, const char* buf, size_t len);

typedef struct {
  const char *content_spec;
  void *user_data;
  _write_func f_write;
} PPMP_Message;

int ppmp_write(PPMP_Message *msg, const char *buf) {
  msg->f_write(msg->user_data, buf, strlen(buf));
  return 0;
}

int ppmp_start(PPMP_Message *msg) {
  ppmp_write(msg, "{");
  ppmp_write(msg, "\"content-spec\": ");
  ppmp_write(msg, MEASUREMENT_V2);
  return 0;
}

int ppmp_finish(PPMP_Message *msg) {
  ppmp_write(msg, "}");
  return 0;
}

int ppmp_write_file(void *_f, const char *buf, size_t len) {
  FILE *f = (FILE*) _f;
  fwrite(buf, len, 1, f);
  return 0;
}

int
ppmp_measurement_payload(PPMP_Message *msg, void *user_data, _write_func writer) {
  msg->content_spec = MEASUREMENT_V2;
  msg->user_data = user_data;
  if (writer) {
    msg->f_write = writer;
  } else {
    msg->user_data = stdout;
    msg->f_write = ppmp_write_file;
  }
  return 0;
}

int ppmp_start_device(PPMP_Message *msg) {
  ppmp_write(msg, ", \"device\": { ");
  return 0;
}

int ppmp_finish_block(PPMP_Message *msg) {
  ppmp_write(msg, "}");
  return 0;
}

int ppmp_write_string(PPMP_Message *msg, const char *str) {
  ppmp_write(msg, "\"");
  ppmp_write(msg, str);
  ppmp_write(msg, "\"");
  return 0;
}

int ppmp_property(PPMP_Message *msg, const char* name, const char *value) {
  ppmp_write_string(msg, name);
  ppmp_write(msg, ": ");
  ppmp_write_string(msg, value);
  return 0;
}

int main(int argc, char *argv[]) {
  PPMP_Message msgbuf;
  PPMP_Message *msg = &msgbuf;
  ppmp_measurement_payload(msg, 0, 0);
  ppmp_start(msg);
  ppmp_start_device(msg);
  ppmp_property(msg, "deviceID", "0213908120380821");
  ppmp_write(msg, ", ");
  ppmp_property(msg, "operationalStatus", "23908402809238");
  ppmp_finish_block(msg);
  ppmp_finish(msg);
}

#if 0
int other_api() {
  PPMP_Device device;
  ppmp_device_init(&device,
		   "deviceID", "309423098423",
		   "operationalStatus", "up",
		   0);
  PPMP_Part part;
  ppmp_part_init(&part,
		 "partTypeID", "");
}
printf("{ \"content-spec\": \"urn:spec://eclipse.org/unide/measurement-message#v2\", "
       "  \"device\": {"
       "    \"deviceID\": \"%s\", "
       "    \"operationalStatus\": \”%s\""
       "  }"
       "}");
#endif
