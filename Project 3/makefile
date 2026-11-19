CXX      := g++
CXXFLAGS := -Wall -std=c++17 -g -fsanitize=address,undefined
SFML     := sfml-graphics sfml-window sfml-system

SFML_CFLAGS = $(shell pkg-config --cflags $(SFML))
SFML_LIBS   = $(shell pkg-config --libs $(SFML))

.PHONY: test run build docs clean

test:
	echo "Hello"

run:
	./sim

SRCS := $(wildcard *.cpp)

build:
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) $(SRCS) -o sim $(SFML_LIBS)

docs:
	doxygen Doxyfile

clean:
	rm -f sim