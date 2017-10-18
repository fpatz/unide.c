.PHONY: all

all: test_measurement buffer_example static_buffer_example

test_measurement: ppmp.o
buffer_example: ppmp.o cbuffer.o
static_buffer_example: ppmp.o
