
include config.mk


MODULES_PATH = $(dir $(wildcard ./src/atema/*/))
MODULES_DIR = $(subst ./src/atema/, ,$(MODULES_PATH))
MODULES = $(MODULES_DIR:/= )


MODULES = core

LIBS = $(addsuffix .a,$(addprefix libatema-,$(MODULES)))
LIBS_PATH = $(addprefix ./lib/,$(LIBS))


%:
	@echo make -C $(addprefix src/atema/,$(@)) MODULE=$(@)
	make -C $(addprefix src/atema/,$(@)) MODULE=$(@)


all: $(MODULES)
	make -C examples




