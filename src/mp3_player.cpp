#include <stdio.h>

#include "hardware/watchdog.h"
#include "pico/stdlib.h"

#include "config.h"
#include "filemanager.h"
#include "player.h"

int main()
{
    stdio_init_all();
    uint8_t vol = 255;
    // getchar();
    puts("starting");

    filemanager::initSd();
    puts("filemanager initted");
    player::init();
    puts("player initted");

    player::play("2.mp3");
    puts("play started!");

    while (!player::isFinished()) {
        char c = getchar_timeout_us(0);
        if (c == 'p') {
            player::togglePause();
            puts("pause toggle");
        } else if (c == '+') {
            vol++;
            puts("vol+");
            player::setVol(vol);

        } else if (c == '-') {
            vol--;
            puts("vol-");
            player::setVol(vol);
        }
        player::tick();
    }

    filemanager::deinitSd();

    puts("Goodbye, world!");
    getchar();
    watchdog_enable(1, 1);
    while (1)
        ;
}
