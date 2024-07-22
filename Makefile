.PHONY

source_files := $(wildcard src/*.c)

build: $(source_files)
	cc
clean:
	rm -rf bulid