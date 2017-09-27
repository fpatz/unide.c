.PHONY: all

all: test_measurement buffer_example

test_measurement: ppmp.o
buffer_example: ppmp.o cbuffer.o
