#include <stdio.h>
#include <stdlib.h>

#include "hardware/watchdog.h"
#include "pico/stdlib.h"

#include "config.h"
#include "filemanager.h"
#include "gui/gui.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "player.h"
#include "playlistfile.h"
#include "queue.h"

void init() {
    stdio_init_all();
    puts("starting");

    filemanager::init();
    puts("filemanager initted");
    player::init();
    puts("player initted");

    multicore_launch_core1(gui::init);
}

void serial_ctrl() {
    char c = getchar_timeout_us(0);
    if (c == 'p') {
        player::togglePause();
        puts("pause toggle");
    } else if (c == '+') {
        player::setVol(player::getVol() + 1);
    } else if (c == '-') {
        player::setVol(player::getVol() - 1);
    } else if (c == 'a') {
        // TODO add functionality to restart current song if its after 5sec of
        // current song
        prev_queue_index();
        player::stop();
    } else if (c == 'd') {
        next_queue_index();
        player::stop();
    } else if (c == 's') {
        printf("shuffel\n");
        shuffle_queue();
    }
}

int main() {
    init();
<<<<<<< HEAD
=======
    // TODO vals to config
    create_queue();

    char *str1 = "Whenever.mp3";
    char *str2 = "Creature.mp3";

    // add_to_queue_end(str1);
    playlistfile::add_to_queue("playlist.m3u");
    // add_to_queue_at(str2, 0);

    // set_queue_index(1);
>>>>>>> fe8ea8a2b3188429ec985694e20045b0a3853491

    puts("init complete");
    while (true) {
        serial_ctrl();
        player::tick();
    }

    filemanager::deinitSd();
    destroy_queue();

    puts("Goodbye, world!");
    getchar();
    watchdog_enable(1, 1);
    while (1)
        ;
}
