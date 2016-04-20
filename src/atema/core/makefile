
#include ../../../config.mk

#TARGET = $(MODULE)
#LIB = $(addsuffix .a,$(addprefix libatema-,$(MODULE)))
#LIBS_PATH = $(addprefix ./lib/,$(LIB))

#error.o: $(subst $@: , ,$(shell g++ -std=c++11 -I../../../include -MM -c $(subst .o,.cpp,$@)))
#	g++ -std=c++11 -I../../../include -c $(subst .o,.cpp,$@) -o $@


SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))

all: $(OBJ)
	@echo Im $(OBJ)

%.o: %.cpp
	g++ -std=c++11 -MMD -I../../../include -c $< -o $@

-include $(SRC:.cpp=.d)


