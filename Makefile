_DEPS = cpcli_utils.hpp \
				cpcli_operations.hpp \
				cpcli_project_config.hpp \
				cpcli_problem_config.hpp \
				color.hpp \
				testlib.h

_OBJ = main.o \
			 cpcli_utils.o \
			 cpcli_operations.o \
			 cpcli_project_config.o \
			 cpcli_problem_config.o

APPBIN = cpcli_app

IDIR = include
CC = g++
CFLAGS = -I$(IDIR) -Wall -Wextra -g -std=c++17
ODIR = obj
SDIR = src
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

CHECKER_DIR = binary/checker
CHECKER_SCR = $(wildcard $(CHECKER_DIR)/*.cpp)
CHECKER_BIN = $(patsubst %.cpp,%,$(CHECKER_SCR))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(APPBIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%: %.cpp
	$(CC) -o $@ $^ $(CFLAGS)

all: $(APPBIN) $(CHECKER_BIN) 

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o 
	rm -f $(APPBIN) $(TESTBIN)
	find $(CHECKER_DIR) -type f ! -name "*.cpp" -exec rm {} \;
