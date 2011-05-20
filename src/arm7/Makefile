ECHO = @echo
RM = -@rm -f

MCFLAGS = $(CFLAGS) -mcpu=arm7tdmi -march=armv4t

OBJECTFILES = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all clean

all: ../arm7elf

../arm7elf: $(OBJECTFILES)
	$(ECHO) "LINK    >$@"
	$(MY_LINK) -e main -Ttext 0x03800000 -n $^ -o $@

%.o: %.c
	$(ECHO) "CC      $<"
	$(MY_CC) $(MCFLAGS) -c $< -o $@

clean:
	$(RM) ../arm7elf $(OBJECTFILES)

