CXX := g++
TARGET := assignment4

ifeq ($(OS), windows)
	LFLAGS := -static -lm -lglut32cu -lglu32 -lopengl32
	CLEAN := del .\obj\*.o $(TARGET)
else
	LFLAGS := -lm -lGL -lGLU -lglut
	CLEAN := rm ./obj/*.o $(TARGET)
endif

CFLAGS := -Wall -Wextra -std=c++17 -c -I./inc

MAKE_OBJ = $(CXX) $< -o $@ $(CFLAGS)
MAKE_EXE = $(CXX) $^ -o $@ $(LFLAGS)

$(TARGET): obj/main.o obj/vec3.o obj/matrix.o obj/stb_image.o
	$(MAKE_EXE)
	
obj/main.o: src/main.cpp inc/vec3.hpp inc/matrix.hpp
	$(MAKE_OBJ)

obj/vec3.o: src/vec3.cpp inc/vec3.hpp
	$(MAKE_OBJ)

obj/matrix.o: src/matrix.cpp inc/matrix.hpp
	$(MAKE_OBJ)

obj/stb_image.o: src/stb_image.c inc/stb_image.h
	$(MAKE_OBJ)

clean:
	$(CLEAN)
