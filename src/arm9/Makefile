ECHO = @echo
RM = -@rm -f

MCFLAGS = $(CFLAGS) -mcpu=arm946e-s -march=armv5te
MASFLAGS = $(ASFLAGS) -mcpu=arm946e-s -march=armv5te -c

OBJECTFILES = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S)) font.o

.PHONY: all clean

all: ../arm9elf

ifeq ("$(GBC_ROM)", "")
../arm9elf: $(OBJECTFILES)
	$(ECHO) "LINK    >$@"
	$(MY_LINK) -e _start -Ttext 0x02000000 -n -L$(shell $(MY_CC) -print-libgcc-file-name | sed -e 's/\(.*\)\/\(.*\)/\1/') $^ -o $@ -lgcc
else
../arm9elf: $(OBJECTFILES) gbc_rom.o
	$(ECHO) "LINK    >@"
	$(MY_LINK) -e _start -Ttext 0x02000000 -n -L$(shell $(MY_CC) -print-libgcc-file-name | sed -e 's/\(.*\)\/\(.*\)/\1/') $^ -o $@ -lgcc

gbc_rom.o: rom.gbc
	$(ECHO) "OCP     $<"
	$(OCP) -I binary -B armv5te -O elf32-littlearm --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

rom.gbc: $(GBC_ROM)
	@cp $< $@
endif

%.o: %.c
	$(ECHO) "CC      $<"
	$(MY_CC) $(MCFLAGS) -c $< -o $@

%.o: %.S
	$(ECHO) "ASM     $<"
	$(MY_CC) $(MASFLAGS) $< -o $@

font.o: font.npf
	$(ECHO) "OCP     $<"
	$(OCP) -I binary -B armv5te -O elf32-littlearm --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

clean:
	$(RM) ../arm9elf $(OBJECTFILES) gbc_rom.o rom.gbc
