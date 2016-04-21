
include config.mk


MODULES_PATH = $(dir $(wildcard ./src/$(NAME)/*/))
MODULES_DIR = $(subst ./src/$(NAME)/, ,$(MODULES_PATH))
MODULES = $(MODULES_DIR:/= )


MODULES = core

LIBS = $(addsuffix .a,$(addprefix lib$(NAME)-,$(MODULES)))
LIBS_PATH = $(addprefix ./lib/,$(LIBS))


%:
	@echo make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@)
	make -C $(addprefix src/$(NAME)/,$(@)) MODULE=$(@) NAME=$(NAME)


all: $(MODULES)
	make -C examples NAME=$(NAME)




