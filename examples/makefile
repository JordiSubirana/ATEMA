

include ../config.mk


toUP = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))


ifeq ($(DYNAMIC_LINK), ON)
	CXXFLAGS += -D$(call toUP,$(NAME))_DYNAMIC
else
	CXXFLAGS += -D$(call toUP,$(NAME))_STATIC
endif


SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))
BIN = $(addprefix ../bin/,$(subst .cpp,.exe,$(SRC)))


%.o: %.cpp
#	@echo $(CXX) $< $(CXXFLAGS)  -I../include -I../extlibs/include -o $@
	$(CXX) $< $(CXXFLAGS)  -I../include -I../extlibs/include -o $@


%.exe: ../examples/%.o
#	@echo $(LD) -L../lib -L../extlibs/lib  $(LDFLAGS) $< ../lib/lib$(NAME)-core.a -o $@
	$(LD) -L../lib -L../extlibs/lib  $(LDFLAGS) $< $(wildcard ../lib/*.a ../extlibs/lib/*.a) -o $@

all: $(BIN)


-include $(SRC:.cpp=.d)


