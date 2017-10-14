This is a first draft of a C API for generating PPMP messages. The
API is designed to be used with constrained devices, i.e. it has a
very modest footprint and does not use the heap. Buffering of messages
is factored out into a callback.

To generate a message, a `PPMP` structure has to be initialized using
`ppmp_init()`::

  #include <ppmp.h>

  ...
  PPMP ppmp;
  ppmp_init(&ppmp, NULL, NULL, PPMP_PRETTY);
  ...

The first argument to `ppmp_init()` is a point to a `PPMP` struct. The
second is a `void *user_data` that is not interpreted by the library,
but passed unmodified to the third argument, that is a callback to
write pieces of the generated message somewhere. The signature of the
callback looks like::

  int callback(void *user_data, const char *buf, int len);

The callback is expected to return 0 when no error has occured. The
library provides a default callback that simply writes the message to
`stdout`. The default is used, when `NULL` is passed for the callback
argument.

The fourth and last argument to `ppmp_init()` is a set of flags. The
only flags available at this time is `PPMP_PRETTY` the formats to be
easily readable for humans.

A message is then constructed by calling APIs in the right order. To
start a measurement payload, the first call on the initialized `PPMP`
structure is `ppmp_measurement_payload()`, and the payload is finished
with `ppmp_finish()`::

  ...
  ppmp_measurement_payload(&ppmp);
  ... /* message data comes here */
  ppmp_finish(&ppmp);
  ...

(The measurement payload is the only one implemented for now)

For each JSON object inside a message, there is a set of calls to
construct the data. All calls take the `PPMP` structure as their first
argument.

The `Device` object is created by `ppmp_device()`. The `Part` object
by `ppmp_part()`. Compare their declarations in `ppmp.h` for their
arguments.

The `"measurements"` array, due to its more complex structure,
requires a handful of different calls. See the example in
`test_measurement.c` for reference.

---

To generate the message into a memory buffer, one can use the
`cbuffer` module in this repository. The callback function is then set
up to write into `cbuffer` object. Compare the example in
`buffer_example.c`.
