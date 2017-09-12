#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define JSON_PRETTY 1

#define JSON_STACK_SIZE 20

typedef int (*write_func)(void *user_data, const char *buf, int len);
typedef struct {
  void *user_data;
  write_func f_write;
  char stack[JSON_STACK_SIZE];
  int stackp;
  char join;
  int flags;
} JSON;

int fwrite_wrap(void *user_data, const char *buf, int len) {
  return (int) fwrite(buf, len, 1, (FILE*) user_data);
}

int json_init(JSON *json, void *user_data, write_func f_write, int flags) {
  json->user_data = user_data;
  json->join = 0;
  json->flags = flags;
  if (f_write == 0) {
    json->user_data = (void*) stdout;
    f_write = fwrite_wrap;
  }
  json->f_write = f_write;
  json->stackp = 0;
  return 0;
}

int json_push(JSON *json, char c) {
  if (json->stackp < JSON_STACK_SIZE) {
    json->stack[json->stackp++] = c;
    return 0;
  }
  return -1;
}

char json_pop(JSON *json) {
  if (json->stackp > 0) {
    return json->stack[--json->stackp];
  }
  return 0;
}

int json_writez(JSON *json, const char *buf) {
  return json->f_write(json->user_data, buf, strlen(buf));
}

int json_writen(JSON *json, ...) {
  va_list vargs;
  const char *buf;
  va_start(vargs, json);
  buf = va_arg(vargs, const char*);
  while (buf) {
    json_writez(json, buf);
    buf = va_arg(vargs, const char*);
  }
  va_end(vargs);
  return 0;
}

int json_string(JSON *json, const char *str) {
  return json_writen(json, "\"", str, "\"", 0);
}

int json_join(JSON *json) {
  char buf[3] = "  ";
  if (json->join) {
    buf[0] = json->join;
    json_writez(json, buf);
  }
  return 0;
}

int json_property(JSON *json, const char *name) {
  json_join(json);
  if (json->flags & JSON_PRETTY) {
    int i;
    json_writez(json, "\n");
    for (i = 0; i < json->stackp; i++) {
      json_writez(json, "  ");
    }
  }
  json_string(json, name);
  json_writez(json, ": ");
  json->join = ',';
  return 0;
}

int json_object(JSON *json, ...) {
  json->join = 0;
  json_writez(json, "{");
  json_push(json, '}');
  return 0;
}

int json_list(JSON *json, ...) {
  json->join = 0;
  json_writez(json, "[");
  json_push(json, ']');
  return 0;
}

int json_end(JSON *json) {
  char buf[2] = " ";
  char c = json_pop(json);
  int i;
  if (json->flags & JSON_PRETTY) {
    json_writez(json, "\n");
    for (i = 0; i < json->stackp; i++) {
      json_writez(json, "  ");
    }
  }
  buf[0] = c;
  json_writez(json, buf);
  json->join = ',';
  return 0;
}

int ppmp_measurement_payload(JSON *json) {
  json_object(json);
  json_property(json, "content-spec");
  json_string(json, "urn:spec://eclipse.org/unide/measurement-message#v2");
  return 0;
}


#define EMIT_PROPERTY(JSON, NAME)		\
  if (NAME) {					\
     json_property(JSON, #NAME);		\
     json_string(JSON, NAME);			\
  }


int ppmp_vmeta(JSON *json, va_list args) {
  const char *key, *value;
  key = va_arg(args, const char*);
  if (key) {
    json_property(json, "metaData");
    json_object(json);
    while (key) {
      value = va_arg(args, const char*);
      json_property(json, key);
      json_string(json, value);
      key = va_arg(args, const char*);
    }
    json_end(json);
  }
  return 0;
}

int ppmp_device(JSON *json, const char *deviceID,
		const char *operationalStatus, ...) {
  va_list args;
  va_start(args, operationalStatus);
  json_property(json, "device");
  json_object(json);
    EMIT_PROPERTY(json, deviceID);
    EMIT_PROPERTY(json, operationalStatus);
    ppmp_vmeta(json, args);
  json_end(json);
  va_end(args);
  return 0;
}

int ppmp_part(JSON *json,
	      const char *partTypeID,
	      const char *partID,
	      const char *result,
	      const char *code,
	      ...) {
  va_list args;
  va_start(args, code);
  json_property(json, "part");
  json_object(json);
    EMIT_PROPERTY(json, partTypeID);
    EMIT_PROPERTY(json, partID);
    EMIT_PROPERTY(json, result);
    EMIT_PROPERTY(json, code);
    ppmp_vmeta(json, args);
  json_end(json);
  va_end(args);
  return 0;
}

int ppmp_measurements(JSON *json) {
  json_property(json, "measurements");
  json_list(json);
  return 0;
}

int ppmp_measurement(JSON *json,
		     const char *ts,
		     const char *result,
		     const char *code) {
  json_object(json);
  EMIT_PROPERTY(json, ts);
  EMIT_PROPERTY(json, result);
  EMIT_PROPERTY(json, code);
  json_property(json, "series");
  json_object(json);
  return 0;
}

int ppmp_offsets(JSON *json, int count, int *offsets) {
  int i;
  char buf[20];
  json_property(json, "$_time");
  json_list(json);
  for (i = 0; i < count; i++) {
    if (i==0) {
      sprintf(buf, "%d", offsets[i]);
    } else {
      sprintf(buf, ", %d", offsets[i]);
    }
    json_writez(json, buf);
  }
  json_end(json);
  return 0;
}

int ppmp_samples(JSON *json, const char *name, int count, double *offsets) {
  int i;
  char buf[20];
  json_property(json, name);
  json_list(json);
  for (i = 0; i < count; i++) {
    if (i==0) {
      sprintf(buf, "%g", offsets[i]);
    } else {
      sprintf(buf, ", %g", offsets[i]);
    }
    json_writez(json, buf);
  }
  json_end(json);
  return 0;
}

int ppmp_finish(JSON *json) {
  while (json->stackp) {
    json_end(json);
  }
  return 0;
}

int main(int argc, const char *argv[]) {
  JSON jsonbuf;
  JSON *json = &jsonbuf;
  int i; 
  json_init(json, NULL, NULL, JSON_PRETTY);

  ppmp_measurement_payload(json);
  ppmp_device(json, "a4927dad-58d4-4580-b460-79cefd56775b", NULL, NULL);

  ppmp_measurements(json);
  for (i = 0; i < 3; i++) {
    ppmp_measurement(json, "2002-05-30T09:30:10.123+02:00", NULL, NULL);
    {
      int offsets[3] = {0, 22, 24};
      double pressure[3] = {24.5, 42.231, 43.12};
      ppmp_offsets(json, 1, offsets);
      ppmp_samples(json, "pressure", 1, pressure);
    }
    json_end(json);
  }
  ppmp_finish(json);

#if 0
  ppmp_device(json,
	      "a4927dad-58d4-4580-b460-79cefd56775b",
	      "UP",
	      "supplier", "Bosch",
	      "lastMaintenance", "2017-08",
	      NULL);

  ppmp_part(json,
	    "a4927dad-58d4-4580-b460-79cefd56775b",
	    "UP",
	    "supplier", "Bosch",
	    "lastMaintenance", "2017-08",
	    NULL);
  json_property(json, "device");
  json_object(json);
  json_property(json, "deviceID");
  json_string(json, "a4927dad-58d4-4580-b460-79cefd56775b");
  json_property(json, "operationalStatus");
  json_string(json, "UP");  
  json_end(json);

  json_property(json, "part");
  json_object(json);
  json_property(json, "partTypeID");
  json_string(json, "FT0823498");
  json_end(json);
#endif
  
  return 0;
}
