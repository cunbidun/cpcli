_DEPS = cpcli_utils.hpp \
				cpcli_operations.hpp \
				cpcli_project_config.hpp \
				cpcli_problem_config.hpp \
				color.hpp

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
CHECKER_DIR = checker
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(APPBIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(CHECKER_DIR)/binary/%:$(CHECKER_DIR)/src/%.cpp
	$(CC) -o $@ $^ $(CFLAGS)

all: $(APPBIN)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o 
	rm -f $(APPBIN) $(TESTBIN)
