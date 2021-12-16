# compiler env.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	CXX=ccache clang++
	LDLIBS=-ldl
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	CXX=ccache clang++
	LDLIBS=-stdlib=libc++
	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
endif

CXXFLAGS=-std=c++2a -MMD -MP -Wall
LDFLAGS=-pthread

# project config
SRC_DIR ?= ./src
INC_DIR=./include
TEST_DIR=./test
LIB_DIR=./lib
BIN_DIR=./bin
OBJ_DIR=./out

# --- source code config --------------
INCS = $(wildcard $(INC_DIR)/*.hpp)

JSON_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
INTEG_SRCS = $(JSON_SRCS) $(TEST_SRCS)

# --- the object files config --------------
JSON_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(JSON_SRCS:.cpp=.o)))
TEST_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
INTEG_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(INTEG_SRCS:.cpp=.o)))

# --- build gtest (integrated) --------
INTEG_TARGET = $(BIN_DIR)/json_test
INTEG_DEPS = $(INTEG_OBJS:.o=.d)

default: $(INTEG_TARGET)
.PHONY: default

#$(INTEG_TARGET): $(INTEG_SRCS)
#	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
#	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(LDLIBS) -o $@ $^ -lgtest_main -lgtest

$(INTEG_TARGET): $(INTEG_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(INTEG_OBJS) -o $@ $(LDLIBS) -lgtest_main -lgtest

$(JSON_OBJS): $(JSON_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c src/$(notdir $(@:.o=.cpp)) -o $@

$(TEST_OBJS): $(TEST_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c test/$(notdir $(@:.o=.cpp)) -o $@


-include $(INTEG_DEPS)


# --- Build for AFW -------------------
JSON_TARGET = $(LIB_DIR)/libjson.a
JSON_DEPS = $(JSON_OBJS:.o=.d)

json: $(JSON_TARGET)
.PHONY: json
CXXFLAGS+= -fPIC #-flto=full

$(JSON_TARGET): $(JSON_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	ar rs $(JSON_TARGET) $(JSON_OBJS)
	ranlib -c $(JSON_TARGET)
#	$(AR) rvs $(JSON_TARGET) $(JSON_OBJS)
#	ranlib -c $@

-include $(JSON_DEPS)


# --- Build for AFW(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	JSON_SO_TARGET = $(LIB_DIR)/libjson.so
endif
ifeq ($(UNAME),Darwin)
	JSON_SO_TARGET = $(LIB_DIR)/libjson.dylib
endif

jsonshared: $(JSON_SO_TARGET)
.PHONY: jsonshared

$(JSON_SO_TARGET): $(JSON_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	$(CXX) $(LDFLAGS) $(JSON_OBJS) $(SHARED_CXXFLAGS) -o $@ $(LDLIBS)

# --- Build for test cases w/libjson.a ---
TEST_TARGET = $(BIN_DIR)/test_with_jsonlib
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LIBS = $(JSON_TARGET)

test: $(TEST_TARGET)
.PHONY: test

$(TEST_TARGET): $(TEST_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(TEST_LDLIBS) $(TEST_OBJS) $(TEST_LIBS) -o $@ -lgtest_main -lgtest

# --- Build for test cases w/libjson.so ---
TEST_SHARED_TARGET = $(BIN_DIR)/test_with_jsonlib_so
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LIBS = $(JSON_SO_TARGET)

testshared: $(TEST_SHARED_TARGET)
.PHONY: testshared

$(TEST_SHARED_TARGET): $(TEST_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(TEST_LDLIBS) $(TEST_OBJS) $(TEST_LIBS) -o $@ -lgtest_main -lgtest



.PHONY: all
all: $(JSON_TARGET) $(JSON_SO_TARGET) $(TEST_TARGET) $(TEST_SHARED_TARGET) $(INTEG_TARGET)

# --- clean up ------------------------
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(INTEG_TARGET) $(OBJS) $(JSON_DEPS) $(TEST_OBJS) $(INTEG_OBJS) $(INTEG_DEPS)
