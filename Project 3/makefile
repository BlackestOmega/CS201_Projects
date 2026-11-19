CXX      := g++
CXXFLAGS := -Wall -std=c++17 -g -fsanitize=address,undefined

ifneq ($(filter conan,$(MAKECMDGOALS)),)
  PKG  := PKG_CONFIG_PATH=build/conan pkg-config
  SFML := sfml-all-graphics
else
  PKG  := pkg-config
  SFML := sfml-graphics sfml-window sfml-system
endif

SFML_CFLAGS = $(shell $(PKG) --cflags $(SFML) 2>/dev/null)
SFML_LIBS   = $(shell $(PKG) --libs $(SFML) 2>/dev/null)

# Two programs, each with its own main():
#   sim   = every .cpp except test_main.cpp
#   tests = every .cpp except main.cpp (SFML files included, so they get tested too)
APP_SRCS  := $(filter-out test_main.cpp,$(wildcard *.cpp))
TEST_SRCS := $(filter-out main.cpp,$(wildcard *.cpp))

# Stop with a clear message if SFML can't be found (instead of a wall of errors)
CHECK_SFML = @$(PKG) --exists $(SFML) || { \
	echo "SFML not found ($(SFML))."; \
	echo "  system: sudo apt install libsfml-dev"; \
	echo "  conan:  make deps, then check that build/conan has .pc files"; \
	exit 1; }

.PHONY: build run test deps docs clean conan

build:
	$(CHECK_SFML)
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) $(APP_SRCS) -o sim $(SFML_LIBS)

run: build
	./sim

test:
	$(CHECK_SFML)
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) $(TEST_SRCS) -o tests $(SFML_LIBS)
	./tests

# Install SFML with Conan into build/conan (needed once before using "conan")
deps:
	uv run conan install . --output-folder=build/conan --build=missing \
		-c tools.system.package_manager:mode=install \
		-c tools.system.package_manager:sudo=True

docs:
	doxygen Doxyfile

clean:
	rm -f sim tests

# "conan" is only a flag, so the target itself does nothing
conan:
	@: