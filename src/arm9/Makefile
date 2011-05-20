ECHO = @echo
RM = -@rm -f

MCFLAGS = $(CFLAGS) -mcpu=arm946e-s -march=armv5te
MASFLAGS = $(ASFLAGS) -march=armv5te

OBJECTFILES = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S)) font.o

.PHONY: all clean

all: ../arm9elf

../arm9elf: $(OBJECTFILES)
	$(ECHO) "LINK    >$@"
	$(MY_LINK) -e _start -Ttext 0x02000000 -n -L$(shell $(MY_CC) -print-libgcc-file-name | sed -e 's/\(.*\)\/\(.*\)/\1/') $^ -o $@ -lgcc

%.o: %.c
	$(ECHO) "CC      $<"
	$(MY_CC) $(MCFLAGS) -c $< -o $@

%.o: %.S
	$(ECHO) "ASM     $<"
	$(MY_AS) $(MASFLAGS) $< -o $@

%.o: font.npf
	$(ECHO) "OCP     $<"
	$(OCP) -I binary -B armv5te -O elf32-littlearm --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

clean:
	$(RM) ../arm9elf $(OBJECTFILES)

