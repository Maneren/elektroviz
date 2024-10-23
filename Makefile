# Copyright (c) 2020-present Caps Collective & contributors
# Originally authored by Jonathan Moallem (@jonjondev) & Aryeh Zinn (@Raelr)
#
# This code is released under an unmodified zlib license.
# For conditions of distribution and use, please see:
#     https://opensource.org/licenses/Zlib

# Define custom functions
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
platformpth = $(subst /,$(PATHSEP),$1)

# Check for Windows
ifeq ($(OS), Windows_NT)
	# Set Windows macros
	platform := Windows
	CXX ?= g++
	linkFlags += -Wl,--allow-multiple-definition -pthread -lopengl32 -lgdi32 -lwinmm -mwindows -static -static-libgcc -static-libstdc++
	THEN := &&
	PATHSEP := \$(BLANK)
	MKDIR := -mkdir -p
	RM := -del /q
	COPY = -robocopy "$(call platformpth,$1)" "$(call platformpth,$2)" $3
else
	# Check for MacOS/Linux
	UNAMEOS := $(shell uname)
	ifeq ($(UNAMEOS), Linux)
		# Set Linux macros
		platform := Linux
		linkFlags += -l GL -l m -l pthread -l dl -l rt -l X11
	endif
	ifeq ($(UNAMEOS), Darwin)
		# Set macOS macros
		platform := macOS
		linkFlags += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
	endif

	# Set UNIX macros
	CXX ?= clang++
	THEN := ;
	PATHSEP := /
	MKDIR := mkdir -p
	RM := rm -rf
	COPY = cp -r $1$(PATHSEP)$3 $2
endif

buildName := release
CXXFLAGS := -g0 -Ofast 
RAYLIB_BUILD_MODE := RELEASE

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	buildName := debug
	CXXFLAGS := -g3 -gdwarf-2 -Og -fno-omit-frame-pointer -Wall -Wextra
	RAYLIB_BUILD_MODE := DEBUG
endif

# Set global macros
buildDir := build
buildTargetDir := $(buildDir)/$(platform)/$(buildName)
buildLibDir := $(buildTargetDir)/lib
executable := elektroviz
target := $(buildTargetDir)/$(executable)
docBuildDir := doc/build
docSource := doc/$(executable).tex
docTarget := $(docBuildDir)/$(executable).pdf
sources := $(call rwildcard,src/,*.cpp)
objects := $(patsubst src/%, $(buildTargetDir)/%, $(patsubst %.cpp, %.o, $(sources)))
depends := $(patsubst %.o, %.d, $(objects))
compileFlags += -std=c++23 -I include
linkFlags = -L $(buildLibDir) -l raylib

# Lists phony targets for Makefile
.PHONY: all setup submodules lib build execute clean package

# Default target, compiles, executes and cleans
all: $(target) execute clean

# Sets up the project for compiling, generates includes and libs
setup: include lib

# Pull and update the the build submodules
submodules:
	git submodule update --init --recursive --depth 1

# Copy the relevant header files into includes
include: submodules
	$(MKDIR) $(call platformpth, ./include)
	$(call COPY,vendor/raylib/src,./include,raylib.h)
	$(call COPY,vendor/raylib/src,./include,raymath.h)
	$(call COPY,vendor/raylib/src,./include,rlgl.h)
	$(call COPY,vendor/raylib-cpp/include,./include,*.hpp)
	$(call COPY,vendor/json/include,./include,nlohmann)

lib: $(buildLibDir)/libraylib.a

# Build the raylib static library file and copy it into lib
$(buildLibDir)/libraylib.a: submodules
	cd vendor/raylib/src $(THEN) "$(MAKE)" clean
	cd vendor/raylib/src $(THEN) "$(MAKE)" raylib \
		PLATFORM=PLATFORM_DESKTOP \
		RAYLIB_BUILD_MODE=$(RAYLIB_BUILD_MODE) \
		RAYLIB_MODULE_AUDIO=FALSE \
		RAYLIB_MODULE_MODELS=FALSE \
		GRAPHICS=GRAPHICS_API_OPENGL_43
	$(MKDIR) $(call platformpth, $(buildLibDir))
	$(call COPY,vendor/raylib/src,$(buildLibDir),libraylib.a)

build: $(target)

# Link the program and create the executable
$(target): $(objects)
	$(CXX) $(objects) -o $(target) $(linkFlags)

# Add all rules from dependency files
-include $(depends)

# Compile objects to the build directory
$(buildTargetDir)/%.o: src/%.cpp Makefile
	$(MKDIR) $(call platformpth, $(@D))
	$(CXX) -MMD -MP -c $(compileFlags) $< -o $@ $(CXXFLAGS)

# Run the executable
execute: $(target)
	$(target) $(ARGS)

# Run the executable with valgrind memcheck
valgrind: $(target)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no $(target) $(ARGS)

# Clean up all relevant files
clean:
	$(RM) $(call platformpth, $(buildDir)/*)
	$(RM) $(call platformpth, ./dist/*)

$(docTarget): $(docSource)
	$(MKDIR) $(docBuildDir)
	tectonic $(docSource) -o $(docBuildDir)

package: $(target) $(docTarget)
	git clone file://$(shell pwd)/.git ./dist
	$(RM) $(call platformpth, ./dist/*)
	$(MKDIR) ./dist/doc
	$(MKDIR) ./dist/bin
	$(call COPY,./$(buildTargetDir),./dist/bin,$(executable))
	$(call COPY,.,./dist,src)
	$(call COPY,.,./dist,Makefile)
	$(call COPY,.,./dist,Dockerfile)
	$(call COPY,.,./dist,scenarios)
	$(call COPY,./scripts,./dist,*)
	$(call COPY,./$(docBuildDir),./dist/doc,*.pdf)
