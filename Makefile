RM = -@rm -f

.PHONY: all clean

all:
	@./build.sh

clean:
	-@$(MAKE) -s -r -C tools clean
	-@$(MAKE) -s -r -C src clean
	$(RM) gbconds.nds
