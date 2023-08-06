#include <stdio.h>
//

#include "f_util.h"
#include "ff.h"
#include "pico/stdlib.h"
#include "rtc.h"
//
#include "hw_config.h"

#include "MP3DecoderMAD.h"

using namespace libmad;

void pcmDataCallback(MadAudioInfo &info, int16_t *pwm_buffer, size_t len) {
    printf("PCM Data: %i Hz, %i Channels, %zu Samples\n", info.sample_rate, info.channels, len);
}

MP3DecoderMAD mp3(pcmDataCallback);

int main() {
    stdio_init_all();
    time_init();
    
    getchar();

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    puts("mounted!");
    FIL fil;
    const char* const filename = "test.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    fr = f_open(&fil, "bbng.mp3", FA_READ);
    char buf[16384];
    unsigned int read_bytes = 0;
    mp3.begin();
    do {
        absolute_time_t bef = get_absolute_time();
        f_read(&fil, buf, sizeof(buf), &read_bytes);
        int diff = absolute_time_diff_us(bef, get_absolute_time()) / 1000;
        printf("bytes read: %u; read time: %i\n", read_bytes, diff);
        mp3.write(buf, read_bytes);
    } while (read_bytes == sizeof(buf));
    fr = f_close(&fil);
    

    f_unmount(pSD->pcName);

    puts("Goodbye, world!");
    for (;;);
}