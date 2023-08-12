#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "config.h"
#include "filemanager.h"
#include "player.h"

int main()
{
    stdio_init_all();
    time_init();

    getchar();

    filemanager.initSd();
    puts("filemanager initted");
    player.init();
    puts("player initted");

    player.play("pufotest.mp3");
    puts("play started!");

    while (!player.isFinished()) {
        if (getchar_timeout_us(0) == 'p') {
            player.togglePause();
            puts("pause toggle");
        }
        player.tick();
    }


    filemanager.deinitSd();

    puts("Goodbye, world!");
    getchar();
    watchdog_enable(1, 1);
    while(1);
}