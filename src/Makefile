ECHO = @echo

MAKEDIR = @$(MAKE) -s -r "CFLAGS=$(CFLAGS)" "ASFLAGS=$(ASFLAGS)" "MY_CC=$(MY_CC)" "MY_AS=$(MY_AS)" "MY_LINK=$(MY_LINK)" "OCP=$(OCP)" -C

SUBDIRS = $(shell ls -p --color=none | grep / | grep -v include)
SUBDIRTARGETS = $(patsubst %/,dir-%,$(SUBDIRS))

.PHONY: all clean

all: $(SUBDIRTARGETS)

clean: $(addprefix clean,$(SUBDIRTARGETS))

dir-%: %
	$(ECHO) "-------- $<"
	$(MAKEDIR) $<

cleandir-%: %
	$(MAKEDIR) $< clean

