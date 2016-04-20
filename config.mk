

############################################
#    C.O.M.M.O.N.  S.E.T.T.I.N.G.S.


# uncomment to compile on debug mode (before reuse, do make clean)
#DEBUG_MODE = ON

# uncomment to compile a dynamic link library (before reuse, do make clean)
#DYNAMIC_LINK = ON







###########################################
#     A.V.A.N.C.E.D.     S.E.T.T.I.N.G.S.


CXX = g++ -c
CXXFLAGS = -std=c++11 -O3 -MMD


LD = g++
LDFLAGS = -std=c++11

AR = ar
ARFLAGS = -q


ifeq ($(DEBUG_MODE), ON)
    CXXFLAGS += -g
endif


ifeq ($(OS),Windows_NT)
    CCFLAGS += -DWIN32
    CCFLAGS += -DWINDOWS
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        CXXCFLAGS += -DAMD64
        CXXCFLAGS += -DATEMA64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        CXXCFLAGS += -DIA32
        CXXCFLAGS += -DATEMA32
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CXXCFLAGS += -DLINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CXXCFLAGS += -DOSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CXXCFLAGS += -DAMD64
        CXXCFLAGS += -DATEMA64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CXXCFLAGS += -DIA32
        CXXCFLAGS += -DATEMA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CXXCFLAGS += -DARM
    endif
endif




ifeq ($(DYNAMIC_LINK), ON)
    # -Wl,--out-implib,libexample_dll.a
    LINK_LIB = $(LD) -shared -fPIC $(LDFLAGS)
    CXXFLAGS += -fPIC -DATEMA_DYNAMIC
else
    LINK_LIB = $(AR) $(ARFLAGS)
endif

