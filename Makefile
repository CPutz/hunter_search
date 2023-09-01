SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin

EXE=$(BIN_DIR)/enum
SRC=$(wildcard $(SRC_DIR)/*.cpp)
OBJ=$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
MOD=gnu

CXX=mpic++
CXXFLAGS=-g -O3 -pedantic -std=c++11 -Wall -Wextra -D DEGREE=$(DEG)
INCLUDE=-I./share/$(MOD)/include -I./include
LDFLAGS=-L./share/$(MOD)/lib -pthread
LDLIBS=-lgmp -ldl

.PHONY: all all-stats clean

all: check-deg
all: $(EXE)

all-stats: check-deg
all-stats: $(EXE)-stats
all-stats: CXXFLAGS+=-D STATS

check-deg:
ifndef DEG
	$(error DEG is undefined)
endif

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(INCLUDE) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE)-stats: $(OBJ) | $(BIN_DIR)
	$(CXX) $(INCLUDE) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)