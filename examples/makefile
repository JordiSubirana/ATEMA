

include ../config.mk


SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))
BIN = $(addprefix ../bin/,$(subst .cpp,.exe,$(SRC)))


%.o: %.cpp
	@echo $(CXX) $< $(CXXFLAGS)  -I../include -I../extlibs/include -o $@
	$(CXX) $< $(CXXFLAGS)  -I../include -I../extlibs/include -o $@


%.exe: ../examples/%.o
	@echo $(LD) -L../lib -L../extlibs/lib  $(LDFLAGS) $< ../lib/libatema-core.a -o $@
	$(LD) -L../lib -L../extlibs/lib  $(LDFLAGS) $< ../lib/libatema-core.a -o $@

all: $(BIN)


-include $(SRC:.cpp=.d)


