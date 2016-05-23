
include config.mk


MODULES_PATH = $(dir $(wildcard ./src/$(NAME)/*/))
MODULES_DIR = $(subst ./src/$(NAME)/, ,$(MODULES_PATH))
MODULES = $(MODULES_DIR:/= )
# MODULES = utility core context window


LIBS = $(addsuffix .a,$(addprefix lib$(NAME)-,$(MODULES)))
LIBS_PATH = $(addprefix ./lib/,$(LIBS))


%:
#	@echo make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@)
	make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@) NAME=$(NAME)


.PHONY: prepare cleana all clean
all: prepare cleana $(MODULES)
	make -C examples NAME=$(NAME)


prepare:
ifeq (,$(wildcard ./bin))
	mkdir bin
endif
ifeq (,$(wildcard ./lib))
	mkdir lib
endif



cleana:
ifeq ($(OS),Windows_NT)
	del /s lib$(NAME)*.a
	del /s lib$(NAME)*.dll
else
	find . -name "lib$(NAME)*.a" -type f -delete
	find . -name "lib$(NAME)*.so" -type f -delete
endif


clean:
ifeq ($(OS),Windows_NT)
	del /s *.o
	del /s lib$(NAME)*.a
	del /s *.d
	del /s lib$(NAME)*.so
	del /s lib$(NAME)*.dll
	del /s *.exe
else
	find . -name "*.o" -type f -delete
	find . -name "lib$(NAME)*.a" -type f -delete
	find . -name "*.d" -type f -delete
	find . -name "lib$(NAME)*.so" -type f -delete
	find . -name "lib$(NAME)*.dll" -type f -delete
	find . -name "*.exe" -type f -delete
endif
