

############################################
#    C.O.M.M.O.N.  S.E.T.T.I.N.G.S.


# uncomment to compile on debug mode (before reuse, do make clean)
DEBUG_MODE = ON

# uncomment to compile a dynamic link library (before reuse, do make clean)
DYNAMIC_LINK = ON

# default target. You can specify the target: make NAME=atema
NAME?=atema


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


