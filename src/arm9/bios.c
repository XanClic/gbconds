#include <stdint.h>

#if 0
void bios_call(int num, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
    __asm__ __volatile__ ("strh %0, [r15, #14];"
                          "mov r0, %1;"
                          "mov r1, %2;"
                          "mov r2, %3;"
                          "mov r3, %4;"
                          "swi 0x000"
                          :: "r"(num), "r"(r0), "r"(r1), "r"(r2), "r"(r3)
                          : "r0", "r1", "r2", "r3");
}
#endif

