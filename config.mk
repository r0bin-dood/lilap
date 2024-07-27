TARGET_EXEC := lilap

BUILD_DIR := ./build
SOURCE_DIR := ./src
INCLUDE_DIR := ./src
EXAMPLE_DIR := ./example

SOURCE_FILES := $(shell find $(SOURCE_DIR) -name '*.c')

INC_DIRS := $(shell find $(SOURCE_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC := gcc
CFLAGS := -O3 -Wall -Wpedantic
CPPFLAGS := $(INC_FLAGS)
LDFLAGS := -lpthread