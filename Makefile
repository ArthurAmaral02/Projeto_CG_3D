CXX      ?= g++
CXXFLAGS ?= -O2 -Wall
CPPFLAGS += -Iinclude
LDLIBS   += -lglui -lglut -lGLU -lGL

SRC    := $(wildcard src/*.cpp)
OBJ    := $(SRC:src/%.cpp=build/%.o)
DEPS   := $(OBJ:.o=.d)
TARGET := curvas

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDLIBS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET) desenhos/mario.obj

clean:
	rm -rf build $(TARGET)

-include $(DEPS)
