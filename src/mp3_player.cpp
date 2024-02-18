#include <stdio.h>
#include <stdlib.h>

#include "hardware/watchdog.h"
#include "pico/stdlib.h"

#include "btn_handler.h"
#include "config.h"
#include "filemanager.h"
#include "pico/time.h"
#include "player.h"
#include "queue.h"

void init() {
    stdio_init_all();
    sleep_ms(200);
    puts("hello");
    // getchar();
    puts("starting");

    filemanager::initSd();
    puts("filemanager initted");
    player::init();
    puts("player initted");

    int files_len = 1024;
    int dirs_len = 1024;
    char *files = (char *)malloc(files_len);
    char *dirs = (char *)malloc(dirs_len);

    filemanager::list_dir("/", files, files_len, dirs, dirs_len);
    printf("%s\n%s\n", files, dirs);

    init_btn_handler(player::getVol());
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
    }
}

int main() {
    init();
    // TODO vals to config
    create_queue();

    char *str1 = "Creature.mp3";
    char *str2 = "Whenever.mp3";

    add_to_queue(str1);
    add_to_queue_at(str2, 0);

    while (true) {
        while (!get_cur_queue()) {
            serial_ctrl();
            update_btns();
        }

        player::play(get_cur_queue());

        while (!player::isFinished()) {
            serial_ctrl();
            update_btns();
            player::tick();
        }
        next_queue_index(true);
    }

    filemanager::deinitSd();
    destroy_queue();

    puts("Goodbye, world!");
    getchar();
    watchdog_enable(1, 1);
    while (1)
        ;
}
