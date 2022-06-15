_MOBJ = main.o

APPBIN = cpcli_app
TESTBIN = cpcli_test

IDIR = include
ODIR = obj
TODIR = obj/test
SDIR = src
TDIR = test

CC = g++
CFLAGS = -I$(IDIR) -g -std=c++17
TFLAGS = -lgtest -lgtest_main -fprofile-arcs -ftest-coverage

# example: include/cpcli_project_config.hpp include/util/color.hpp include/nlohmann/json.hpp include/testlib.h
INC = $(shell find include -type f -name "*.hpp" -o -name "*.h")

# example: src/test/test.cpp src/cpcli_operations.cpp 
SRC = $(shell find $(SDIR) -type f -name "*.cpp" ! -name "*main.cpp")
TSRC = $(shell find $(TDIR) -type f -name "*.cpp")

# example: obj/test/test.cpp obj/cpcli_operations.cpp 
_OBJ = $(patsubst $(SDIR)%,$(ODIR)%,$(SRC))
_TOBJ = $(patsubst $(TDIR)%,$(TODIR)%,$(TSRC))

# example: obj/test/test.o obj/cpcli_operations.o
OBJ = $(patsubst %.cpp, %.o, $(_OBJ))
TOBJ = $(patsubst %.cpp, %.o, $(_TOBJ))

# example: src/test src
SRC_DIRNAME = $(shell find $(SDIR) -type f -name "*.cpp" -exec dirname {} \; | sort -u)
TSRC_DIRNAME = $(shell find $(TDIR) -type f -name "*.cpp" -exec dirname {} \; | sort -u)

# example: obj/test obj 
OBJ_DIRNAME= $(patsubst $(SDIR)%,$(ODIR)%,$(SRC_DIRNAME))
TOBJ_DIRNAME= $(patsubst $(TDIR)%,$(TODIR)%,$(TSRC_DIRNAME))

CHECKER_DIR = binary/checker

# example: binary/checker/double4.cpp binary/checker/double6.cpp
CHECKER_SCR = $(wildcard $(CHECKER_DIR)/*.cpp)

# example: binary/checker/double4 binary/checker/double6
CHECKER_BIN = $(patsubst %.cpp,%,$(CHECKER_SCR))

# example: obj/main.o
MOBJ = $(patsubst %,$(ODIR)/%,$(_MOBJ))

mkdir:
	mkdir -p $(OBJ_DIRNAME) $(TOBJ_DIRNAME)

# object files
$(OBJ): $(ODIR)%.o : $(SDIR)%.cpp src/main.cpp $(INC) mkdir
	$(CC) -c -o $@ $< $(CFLAGS)

$(MOBJ): src/main.cpp $(INC) mkdir
	$(CC) -c -o $@ $< $(CFLAGS)

$(TOBJ): $(TODIR)%.o : $(TDIR)%.cpp $(INC) mkdir
	$(CC) -c -o $@ $< $(CFLAGS)

# binary
$(APPBIN): $(OBJ) $(MOBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

$(TESTBIN): $(OBJ) $(TOBJ)  
	$(CC) -o $@ $^ $(CFLAGS) $(TFLAGS)

competitive_companion:
	@echo "running 'npm install' in cc"
	cd cc && npm install

install:
	mv cpcli_app ~/.local/bin

# compile checker binary
$(CHECKER_BIN): % : %.cpp
	$(CC) -o $@ $^ $(CFLAGS)

all: $(APPBIN) $(TESTBIN) $(CHECKER_BIN) competitive_companion 

debug: 
	@echo "include dir: $(IDIR)"
	@echo "source files: $(SRC)"
	@echo "test source files: $(TSRC)"
	@echo "obj files: $(OBJ)"
	@echo "test obj files: $(TOBJ)"
	@echo "SRC_DIRNAME: $(SRC_DIRNAME)"
	@echo "TSRC_DIRNAME: $(TSRC_DIRNAME)"
	@echo "OBJ_DIRNAME: $(OBJ_DIRNAME)"
	@echo "TOBJ_DIRNAME: $(TOBJ_DIRNAME)"

.PHONY: clean

clean:
	git clean -dfX $(ODIR) 
	git clean -dfX $(CHECKER_DIR)
	git clean -dfX binary 
	rm -f $(APPBIN) $(TESTBIN)
	rm -f ~/.local/bin/cpcli_app
	rm -rf cc/node_modules
