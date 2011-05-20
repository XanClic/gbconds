#define _POSIX_C_SOURCE 42

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static const uint16_t crc16_poly[8] =
{
    0xC0C1,
    0xC181,
    0xC301,
    0xC601,
    0xCC01,
    0xD801,
    0xF001,
    0xA001
};

uint16_t crc16(uint32_t initial, void *start, size_t length)
{
    uint8_t *buf = start;

    for (int i = 0; i < (int)length; i++)
    {
        initial ^= buf[i];
        for (int j = 0; j < 8; j++)
        {
            if (!(initial & 1))
                initial >>= 1;
            else
                initial = (initial >> 1) ^ (crc16_poly[j] << (7 - j));
        }
    }

    return initial;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Benutzung: crc16 <Datei>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        perror("Datei konnte nicht geöffnet werden.\n");
        return 1;
    }

    struct stat stt;
    fstat(fileno(fp), &stt);
    void *mem = malloc(stt.st_size);
    fread(mem, stt.st_size, 1, fp);
    fclose(fp);

    printf("0x%04X\n", (unsigned int)crc16(0xFFFF, mem, stt.st_size));

    free(mem);

    return 0;
}

