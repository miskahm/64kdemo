# Simple Makefile for Vulkan Demo
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc -I/usr/include/vulkan -I/usr/include/GLFW
LDFLAGS = -lvulkan -lglfw -lm

SOURCES = src/main.c src/vulkan_setup.c src/shader_loader.c src/shadertoy_compat.c
TARGET = Vulkan64KDemo

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean