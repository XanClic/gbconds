#include <io.h>
#include <panic.h>

void ipc_init(void)
{
    vw8(IO, 0x181, 0);
    vw16(IO, 0x184, 0xC008);
}

void ipc_sync(void)
{
    if (vr8(IO, 0x185) & 1)
    {
        if (vr8(IO, 0x184) & 2)
            panic();
        vw32(IO, 0x188, 0);
    }

    while (vr8(IO, 0x185) & 1);

    if (vr32(IO, 0x100000))
        panic();
}

