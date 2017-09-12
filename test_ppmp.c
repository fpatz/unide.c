#include "ppmp.h"

int main() {
  PPMP msg;
  ppmp_init(&msg, PPMP_MEASUREMENT, NULL, NULL, PPMP_PRETTY);
  ppmp_device(&msg, "6e7807d0-5491-11e6-9d32-02423234b390", NULL, NULL);
  ppmp_measurement(&msg, "2002-05-30T09:30:10.123+02:00", NULL, NULL, NULL);
  ppmp_offsets(&msg);
  ppmp_offset(&msg, 0);
  ppmp_offset(&msg, 22);
  ppmp_offset(&msg, 24);
  ppmp_samples(&msg, "pressure");
  ppmp_sample(&msg, 24.3);
  ppmp_sample(&msg, 22.4);
  ppmp_sample(&msg, 23.8);
  ppmp_finish(&msg);
}
