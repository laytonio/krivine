krivine: main.c krivine.h
	cc $< -o $@ -Wall -g -O3

.PHONY: all
all: krivine
