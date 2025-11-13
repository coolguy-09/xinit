# Kconfig Makefile

KCFLAGS = 
KLDFLAGS = 

LXSRC = scripts/kconfig/lxdialog/checklist.c scripts/kconfig/lxdialog/inputbox.c scripts/kconfig/lxdialog/menubox.c scripts/kconfig/lxdialog/textbox.c scripts/kconfig/lxdialog/util.c scripts/kconfig/lxdialog/yesno.c
LXOBJ = scripts/kconfig/lxdialog/checklist.o scripts/kconfig/lxdialog/inputbox.o scripts/kconfig/lxdialog/menubox.o scripts/kconfig/lxdialog/textbox.o scripts/kconfig/lxdialog/util.o scripts/kconfig/lxdialog/yesno.o
KSRC = scripts/kconfig/confdata.c scripts/kconfig/expr.c scripts/kconfig/lexer.lex.c scripts/kconfig/menu.c scripts/kconfig/mnconf-common.c scripts/kconfig/parser.tab.c scripts/kconfig/preprocess.c scripts/kconfig/symbol.c scripts/kconfig/util.c
KOBJ = scripts/kconfig/confdata.o scripts/kconfig/expr.o scripts/kconfig/lexer.lex.o scripts/kconfig/menu.o scripts/kconfig/mnconf-common.o scripts/kconfig/parser.tab.o scripts/kconfig/preprocess.o scripts/kconfig/symbol.o scripts/kconfig/util.o

all: configcheck output/fastrc output/fastctl

scripts/kconfig/%.o: scripts/kconfig/%.c
	@echo "  HOSTCC  $@"
	@$(CC) $(KCFLAGS) -c $< -o $@

scripts/kconfig/lxdialog/%.o: scripts/kconfig/lxdialog/%.c
	@echo "  HOSTCC  $@"
	@$(CC) $(KCFLAGS) -c $< -o $@

scripts/kconfig/mconf: $(LXOBJ) $(KOBJ) scripts/kconfig/mconf.o
	@echo "  HOSTLD  $@"
	@$(CC) $(KLDFLAGS) -o $@ $^ -lncurses

scripts/kconfig/conf: $(KOBJ) scripts/kconfig/conf.o
	@echo "  HOSTLD  $@"
	@$(CC) $(KLDFLAGS) -o $@ $^
