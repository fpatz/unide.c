#include "ppmp.h"

int main(int argc, const char *argv[]) {
  PPMP ppmpbuf;
  PPMP *ppmp = &ppmpbuf;
  int i; 
  ppmp_init(ppmp, NULL, NULL, PPMP_PRETTY);

  ppmp_measurement_payload(ppmp);
  ppmp_device(ppmp, "a4927dad-58d4-4580-b460-79cefd56775b", NULL, NULL);

  ppmp_measurements(ppmp);
  for (i = 0; i < 3; i++) {
    ppmp_measurement(ppmp, "2002-05-30T09:30:10.123+02:00", NULL, NULL);
    {
      int offsets[3] = {0, 22, 24};
      double pressure[3] = {24.5, 42.231, 43.12};
      ppmp_offsets(ppmp, 1, offsets);
      ppmp_samples(ppmp, "pressure", 1, pressure);
    }
    ppmp_end(ppmp);
  }
  ppmp_finish(ppmp);  
  return 0;
}
