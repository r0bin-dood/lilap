TARGET_EXEC := lilap

BUILD_DIR := ./build
SOURCE_DIR := ./src
INCLUDE_DIR := ./src
EXAMPLE_DIR := ./example

SOURCE_FILES := $(shell find $(SOURCE_DIR) -name '*.c')

CC := gcc
CFLAGS := -O3
