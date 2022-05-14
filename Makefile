APPBIN = cpcli_app

IDIR = include
ODIR = obj
SDIR = src

CC = g++
CFLAGS = -I$(IDIR) -Wall -Wextra -g -std=c++17


# example: include/cpcli_project_config.hpp include/util/color.hpp include/nlohmann/json.hpp include/testlib.h
INC = $(shell find include -type f -name "*.hpp" -o -name "*.h")

# example: src/test/test.cpp src/cpcli_operations.cpp 
SRC = $(shell find $(SDIR) -type f -name "*.cpp")

# example: obj/test/test.cpp obj/cpcli_operations.cpp 
_OBJ = $(patsubst $(SDIR)%,$(ODIR)%,$(SRC))

# example: obj/test/test.o obj/cpcli_operations.o
OBJ = $(patsubst %.cpp, %.o, $(_OBJ))

# example: src/test src
SRC_DIRNAME = $(shell find src -type f -name "*.cpp" -exec dirname {} \; | sort -u)

# example: obj/test obj 
OBJ_DIRNAME= $(patsubst $(SDIR)%,$(ODIR)%,$(SRC_DIRNAME))

CHECKER_DIR = binary/checker

# example: binary/checker/double4.cpp binary/checker/double6.cpp
CHECKER_SCR = $(wildcard $(CHECKER_DIR)/*.cpp)

# example: binary/checker/double4 binary/checker/double6
CHECKER_BIN = $(patsubst %.cpp,%,$(CHECKER_SCR))

mkdir:
	mkdir -p $(OBJ_DIRNAME)

$(OBJ): $(ODIR)%.o : $(SDIR)%.cpp $(INC) mkdir
	$(CC) -c -o $@ $< $(CFLAGS)

$(APPBIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

precompiled_headers:
	@echo "compiling headers..."
	binary/precompiled_headers/gen.sh ./project_config.json

competitive_companion:
	@echo "running 'npm install' in cc"
	cd cc && npm install

install:
	mv cpcli_app ~/.local/bin

# compile checker binary
$(CHECKER_BIN): % : %.cpp
	$(CC) -o $@ $^ $(CFLAGS)

all: $(APPBIN) $(CHECKER_BIN) precompiled_headers competitive_companion 

debug: 
	@echo "include dir: $(IDIR)"
	@echo "headers files: $(INC)"
	@echo "source files: $(SRC)"
	@echo "obj files: $(OBJ)"
	@echo "SRC_DIRNAME: $(SRC_DIRNAME)"
	@echo "OBJ_DIRNAME: $(OBJ_DIRNAME)"

.PHONY: clean

clean:
	git clean -dfX $(ODIR) 
	git clean -dfX $(CHECKER_DIR) 
	git clean -dfX binary 
	rm -f $(APPBIN)
