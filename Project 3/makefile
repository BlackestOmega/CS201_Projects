CXX      := g++
CXXFLAGS := -Wall -std=c++17 -g -fsanitize=address,undefined
SFML     := sfml-graphics sfml-window sfml-system

# Use Conan's SFML if it's been installed, otherwise fall back to system SFML
CONAN_DIR := build/conan
ifneq ($(wildcard $(CONAN_DIR)/sfml-graphics.pc),)
  export PKG_CONFIG_PATH := $(CONAN_DIR)
endif

SFML_CFLAGS = $(shell pkg-config --cflags $(SFML))
SFML_LIBS   = $(shell pkg-config --libs $(SFML))

.PHONY: test run build deps clean

test:
	echo "Hello"

run:
	./sim

SRCS := $(wildcard *.cpp)

build:
> $(CXX) $(CXXFLAGS) $(SFML_CFLAGS) $(SRCS) -o sim $(SFML_LIBS)

deps:
	uv run conan install . --output-folder=$(CONAN_DIR) --build=missing \
		-c tools.system.package_manager:mode=install \
		-c tools.system.package_manager:sudo=True

clean:
	rm -f sim