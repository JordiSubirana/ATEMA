

include ../../../config.mk

#TARGET = $(MODULE)
#LIB = $(addsuffix .a,$(addprefix libatema-,$(MODULE)))
#LIBS_PATH = $(addprefix ./lib/,$(LIB))


SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))


ifeq ($(DYNAMIC_LINK), ON)
	ifeq ($(OS),Windows_NT)
		LINK_LIB += -o ../../../lib/lib$(MODULE).dll -Wl,--out-implib,../../../lib$(MODULE)-dll.a
	else
		LINK_LIB += -o ../../../lib/lib$(MODULE).so
	endif
else
	LINK_LIB += -o ../../../lib/lib$(MODULE).a
endif


all: $(OBJ)
	$(LINK_LIB) $(OBJ)
	@echo $(LINK_LIB) $(OBJ)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@
	@echo $(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@

-include $(SRC:.cpp=.d)


