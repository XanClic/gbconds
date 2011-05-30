#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

void mem_write8(uint16_t addr, uint8_t value);
void mem_write16(uint16_t addr, uint16_t value);
uint8_t mem_read8(uint16_t addr);
uint16_t mem_read16(uint16_t addr);
void init_vm_memory(void);

#endif
