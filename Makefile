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
TARGET := agent.exe
TEST_TARGET := tests.exe
CXXFLAGS := -std=c++1y -Wall -Wextra -pedantic -Isrc -Itest
LIBFLAGS := 
SRCS := main.cpp
TEST_SRCS := catch.cpp


CXX_RELEASE := g++
CXXFLAGS_RELEASE := $(CXXFLAGS) -O3
CXX_DEBUG := g++
CXXFLAGS_DEBUG := $(CXXFLAGS) -g

BUILD_DIR := ./build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug
TEST_BUILD_DIR := $(BUILD_DIR)/test
SRC_DIR := ./src
TEST_DIR := ./test
OBJS_RELEASE := $(SRCS:%.cpp=$(RELEASE_DIR)/%.o)
DEPS_RELEASE := $(OBJS_RELEASE:.o=.d)
OBJS_DEBUG := $(SRCS:%.cpp=$(DEBUG_DIR)/%.o)
DEPS_DEBUG := $(OBJS_DEBUG:.o=.d)
OBJS_TEST := $(TEST_SRCS:%.cpp=$(TEST_BUILD_DIR)/%.o)
DEPS_TEST := $(OBJS_TEST:.o=.d)


all: release
	cp $(RELEASE_DIR)/$(TARGET) $(TARGET)

.PHONY: release debug test clean
release: $(RELEASE_DIR) $(RELEASE_DIR)/$(TARGET)
debug: $(DEBUG_DIR) $(DEBUG_DIR)/$(TARGET)
test: $(TEST_BUILD_DIR) $(TEST_BUILD_DIR)/$(TEST_TARGET)
	$(TEST_BUILD_DIR)/$(TEST_TARGET)
clean:
	rm -rf $(BUILD_DIR) $(TARGET)


$(RELEASE_DIR):
	mkdir -p $@
$(RELEASE_DIR)/$(TARGET): $(OBJS_RELEASE)
	$(CXX_RELEASE) $(CXXFLAGS_RELEASE) $^ -o $@ $(LIBFLAGS)
-include $(DEPS_RELEASE)
$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX_RELEASE) $(CXXFLAGS_RELEASE) -MMD -c $< -o $@


$(DEBUG_DIR):
	mkdir -p $@
$(DEBUG_DIR)/$(TARGET): $(OBJS_DEBUG)
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) $^ -o $@ $(LIBFLAGS)
-include $(DEPS_DEBUG)
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) -MMD -c $< -o $@


$(TEST_BUILD_DIR):
	mkdir -p $@
$(TEST_BUILD_DIR)/$(TEST_TARGET): $(OBJS_TEST)
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) $^ -o $@ $(LIBFLAGS)
-include $(DEPS_TEST)
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX_DEBUG) $(CXXFLAGS_DEBUG) -MMD -c $< -o $@
