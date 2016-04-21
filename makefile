
include config.mk


MODULES_PATH = $(dir $(wildcard ./src/$(NAME)/*/))
MODULES_DIR = $(subst ./src/$(NAME)/, ,$(MODULES_PATH))
MODULES = $(MODULES_DIR:/= )


MODULES = core

LIBS = $(addsuffix .a,$(addprefix lib$(NAME)-,$(MODULES)))
LIBS_PATH = $(addprefix ./lib/,$(LIBS))


%:
#	@echo make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@)
	make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@) NAME=$(NAME)


all: $(MODULES)
	make -C examples NAME=$(NAME)


clean:
ifeq ($(OS),Windows_NT)
	del /s *.o
	del /s libatema*.a
	del /s *.d
	del /s libatema*.so
	del /s libatema*.dll
	del /s *.exe
else
	find . -name "*.o" -type f -delete
	find . -name "*.a" -type f -delete
	find . -name "*.d" -type f -delete
	find . -name "*.so" -type f -delete
	find . -name "*.dll" -type f -delete
	find . -name "*.exe" -type f -delete
endif
