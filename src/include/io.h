#ifndef _IO_H
#define _IO_H

#include <config.h>
#include <stdint.h>

#define vw8(b, o, v) *((volatile uint8_t *)(b) + (o)) = (v)
#define vw16(b, o, v) *((volatile uint16_t *)((uint8_t *)(b) + (o))) = (v)
#define vw32(b, o, v) *((volatile uint32_t *)((uint8_t *)(b) + (o))) = (v)

#define vr8(b, o) *((volatile uint8_t *)(b) + (o))
#define vr16(b, o) *((volatile uint16_t *)((uint8_t *)(b) + (o)))
#define vr32(b, o) *((volatile uint32_t *)((uint8_t *)(b) + (o)))

#define IO 0x04000000

#endif

