CC = @gcc
ECHO = @echo
RM = -@rm -f

CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -O3 -g2

.PHONY: all clean

all: crc16

%: %.c
	$(ECHO) "CC      $<"
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) crc16

