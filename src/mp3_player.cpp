#include <stdio.h>

#include "hardware/watchdog.h"
#include "pico/stdlib.h"

#include "config.h"
#include "filemanager.h"
#include "player.h"
#include "btn_handler.h"


void init()
{
    stdio_init_all();
    // getchar();
    puts("starting");

    filemanager::initSd();
    puts("filemanager initted");
    player::init();
    puts("player initted");

    init_btn_handler(player::getVol());
}

void serial_ctrl()
{
    char c = getchar_timeout_us(0);
    if (c == 'p') {
        player::togglePause();
        puts("pause toggle");
    } else if (c == '+') {
        puts("vol+");
        player::setVol(player::getVol() + 1);

    } else if (c == '-') {
        puts("vol-");
        player::setVol(player::getVol() - 1);
    }
}

int main()
{
    init();

    player::play("2.mp3");
    puts("play started!");

    while (!player::isFinished()) {
        serial_ctrl();
        update_btns();
        player::tick();
    }

    filemanager::deinitSd();

    puts("Goodbye, world!");
    getchar();
    watchdog_enable(1, 1);
    while (1)
        ;
}
