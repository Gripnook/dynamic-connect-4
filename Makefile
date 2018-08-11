# Makefile template for compiling C++ projects.
# 
# USAGE:
# 	make [target]
# 
# TARGETS:
# 	release		Release build
# 	debug		Debug build
# 	clean		Clean up the object files
#
# Author: Andrei Purcarus


# Configuration Settings
TARGET := dc4.so
CXXFLAGS := -std=c++1y -Wall -Wextra -pedantic -Isrc -I/usr/include/python3.5 -pthread
LIBFLAGS := -lboost_python-py35
SRCS := ml/main.cpp game/game.cpp game/state.cpp game/drawboard.cpp
DIRECTORIES := game game/heuristics search util ml


CXX_RELEASE := g++
CXXFLAGS_RELEASE := $(CXXFLAGS) -O3
CXX_DEBUG := g++
CXXFLAGS_DEBUG := $(CXXFLAGS) -g

BUILD_DIR := ./build
RELEASE_DIR := $(BUILD_DIR)/release
RELEASE_DIRS := $(DIRECTORIES:%=$(RELEASE_DIR)/%)
DEBUG_DIR := $(BUILD_DIR)/debug
DEBUG_DIRS := $(DIRECTORIES:%=$(DEBUG_DIR)/%)
SRC_DIR := ./src
OBJS_RELEASE := $(SRCS:%.cpp=$(RELEASE_DIR)/%.o)
DEPS_RELEASE := $(OBJS_RELEASE:.o=.d)
OBJS_DEBUG := $(SRCS:%.cpp=$(DEBUG_DIR)/%.o)
DEPS_DEBUG := $(OBJS_DEBUG:.o=.d)


all: release
	cp $(RELEASE_DIR)/$(TARGET) $(TARGET)
	python3 main.py

.PHONY: release debug clean
release: $(RELEASE_DIR) $(RELEASE_DIRS) $(RELEASE_DIR)/$(TARGET)
debug: $(DEBUG_DIR) $(DEBUG_DIRS) $(DEBUG_DIR)/$(TARGET)
clean:
	rm -rf $(BUILD_DIR) $(TARGET)


$(RELEASE_DIR):
	mkdir -p $@
$(RELEASE_DIRS):
	mkdir -p $@
$(RELEASE_DIR)/$(TARGET): $(OBJS_RELEASE)
	$(CXX_RELEASE) $(CXXFLAGS_RELEASE) $^ -shared -o $@ $(LIBFLAGS)
-include $(DEPS_RELEASE)
$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX_RELEASE) $(CXXFLAGS_RELEASE) -MMD -c -fpic $< -o $@


$(DEBUG_DIR):
	mkdir -p $@
$(DEBUG_DIRS):
	mkdir -p $@
$(DEBUG_DIR)/$(TARGET): $(OBJS_DEBUG)
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) $^ -shared -o $@ $(LIBFLAGS)
-include $(DEPS_DEBUG)
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) -MMD -c -fpic $< -o $@

