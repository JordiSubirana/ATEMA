

include ../../../config.mk


SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))

LIB = lib$(NAME)-$(MODULE)



ifeq ($(DYNAMIC_LINK), ON)
  LINK_LIB = $(LD) -shared -fPIC $(LDFLAGS)
  CXXFLAGS += -fPIC -DATEMA_DYNAMIC

  ifeq ($(OS),Windows_NT)
    LINK_LIB += -o ../../../bin/$(LIB).dll -Wl,--out-implib,../../../$(LIB)-dll.a
  else
    LINK_LIB += -o ../../../lib/$(LIB).so
  endif
else
  LINK_LIB = $(AR) $(ARFLAGS) ../../../lib/$(LIB).a
endif


all: $(OBJ)
	@echo $(LINK_LIB) $(OBJ)
	$(LINK_LIB) $(OBJ)


%.o: %.cpp
	@echo $(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@
	$(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@


-include $(SRC:.cpp=.d)

