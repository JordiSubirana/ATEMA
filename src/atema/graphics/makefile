

include ../../../config.mk
-include makefile.dep

# ajoute les sources de la lib
SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))

# ajoute les dependances cpp
SRC_DEP = $(foreach dep,$(DEPENDS),$(wildcard ../../../extlibs/src/$(dep)/*.c) $(wildcard ../../../extlibs/src/$(dep)/*.cpp))
OBJ += $(subst .c,.o,$(subst .cpp,.o,$(SRC_DEP)))



toUP = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

LIB = lib$(NAME)-$(MODULE)



ifeq ($(DYNAMIC_LINK), ON)
  LINK_LIB = $(LD) -shared -fPIC $(LDFLAGS)
  CXXFLAGS += -fPIC -D$(call toUP,$(NAME))_DYNAMIC

  ifeq ($(OS),Windows_NT)
    LINK_LIB += -o ../../../bin/$(LIB).dll -Wl,--out-implib,../../../lib/$(LIB)-dll.a
  else
    LINK_LIB += -o ../../../lib/$(LIB).so
  endif
else
  LINK_LIB = $(AR) $(ARFLAGS) "../../../lib/$(LIB).a"
  CXXFLAGS += -D$(call toUP,$(NAME))_STATIC
endif


CXXFLAGS += -D$(call toUP,$(NAME))_$(call toUP,$(MODULE))_EXPORT


all: $(OBJ)
#	@echo $(LINK_LIB) $(OBJ)
	$(LINK_LIB) $(OBJ)


%.o: %.c
#	@echo $(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@
	$(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@

%.o: %.cpp
#	@echo $(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@
	$(CXX) $< $(CXXFLAGS)  -I../../../include -I../../../extlibs/include -o $@


-include $(SRC:.cpp=.d)

