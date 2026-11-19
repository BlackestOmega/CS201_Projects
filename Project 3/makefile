CXX      := g++
CXXFLAGS := -Wall -std=c++17
SFML     := sfml-graphics sfml-window sfml-system

SFML_CFLAGS = $(shell pkg-config --cflags $(SFML))
SFML_LIBS   = $(shell pkg-config --libs $(SFML))

.PHONY: test run build clean

test:
	echo "Hello"

run:
	./sim

build:
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) main.cpp -o sim $(SFML_LIBS)

clean:
	rm -f sim