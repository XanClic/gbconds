#include <ipc.h>

#include <nds7/touchscreen.h>

int main(void)
{
    init_touchscreen();

    // while (!touchscreen_raw_x());

    ipc_init();
    ipc_sync();

    for (;;);
}

