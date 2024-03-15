#include "display.h"

#include "config.h"
#include "font.h"

#include "hardware/i2c.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include <cstdint>
#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <stdio.h>

#define SSD1306_HEIGHT 64
#define SSD1306_WIDTH 128

#define SSD1306_I2C_ADDR _u(0x3C)

// 400 is usual, but often these can be overclocked to improve display response.
// Tested at 1000 on both 32 and 84 pixel height devices and it worked.
#define SSD1306_I2C_CLK 1000
// #define SSD1306_I2C_CLK             1000

// commands (see datasheet)
#define SSD1306_SET_MEM_MODE _u(0x20)
#define SSD1306_SET_COL_ADDR _u(0x21)
#define SSD1306_SET_PAGE_ADDR _u(0xb0)
#define SSD1306_SET_HORIZ_SCROLL _u(0x26)
#define SSD1306_SET_SCROLL _u(0x2E)

#define SSD1306_NOP _u(0xE3)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST _u(0x81)
#define SSD1306_SET_CHARGE_PUMP _u(0x8D)

#define SSD1306_SET_SEG_REMAP _u(0xA0)
#define SSD1306_SET_ENTIRE_ON _u(0xA4)
#define SSD1306_SET_ALL_ON _u(0xA5)
#define SSD1306_SET_NORM_DISP _u(0xA6)
#define SSD1306_SET_INV_DISP _u(0xA7)
#define SSD1306_SET_MUX_RATIO _u(0xA8)
#define SSD1306_SET_DISP _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV _u(0xD5)
#define SSD1306_SET_PRECHARGE _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG _u(0xDA)
#define SSD1306_SET_VCOM_DESEL _u(0xDB)

#define SSD1306_PAGE_HEIGHT _u(8)
#define SSD1306_NUM_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_WRITE_MODE _u(0xFE)
#define SSD1306_READ_MODE _u(0xFF)

namespace display {
uint8_t frameBuffer[SSD1306_BUF_LEN];

void SSD1306_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i = 0; i < num; i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the
    // control byte to the beginning

    uint8_t *temp_buf = (uint8_t *)malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buf, buflen);

    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, temp_buf, buflen + 1,
                       false);

    free(temp_buf);
}

void SSD1306_send_page(uint8_t buf[]) {
    uint8_t temp_buf[129];
    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buf, 128);
    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, temp_buf, sizeof temp_buf,
                       false);
}

void SSD1306_init() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer


    // SSD1306_send_cmd_list(cmds, count_of(cmds));
    uint8_t cmds[] = {
        SSD1306_SET_DISP_OFFSET, // set display offset
        0x00,                    // no offset
        SSD1306_SET_DISP_START_LINE | 0x00,

        SSD1306_SET_SEG_REMAP | 0x01,
        SSD1306_SET_COM_OUT_DIR |
            0x08, // set COM (common) output scan direction. Scan from bottom
                  // up, COM[N-1] to COM0

        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV, // set display clock divide ratio
        0x80,                     // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,    // set pre-charge period
        0xF1,                     // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,   // set VCOMH deselect level
        0x30,                     // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST, // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON, // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP, // set normal (not inverted) display
                               // Vcc internally generated on our board
        SSD1306_SET_SCROLL |
            0x00, // deactivate horizontal scrolling if set. This is necessary
                  //   as memory writes will corrupt if scrolling was enabled

        SSD1306_SET_DISP | 0x01, // turn display on
        SSD1306_SET_CHARGE_PUMP, // set charge pump
        0x14,
    };

    if (config::HAS_SH1106) {
        cmds[count_of(cmds) - 1] = SSD1306_NOP;
        cmds[count_of(cmds) - 2] = SSD1306_NOP;
    }

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

static void WriteChar(int16_t x, int16_t y, const uint8_t ch) {
    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / 8;

    int idx = ch;
    int fb_idx = y * 128 + x;

    for (int i = 0; i < 5; i++) {
        frameBuffer[fb_idx++] = font[idx * 5 + i];
    }
}

static void WriteString(int16_t x, int16_t y, const char *str) {
    // Cull out any string off the screen
    char *str_ptr = (char *)str;
    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    while (*str_ptr) {
        WriteChar(x, y, *str_ptr++);
        x += 8;
    }
}

void render() {
    uint8_t cmds[] = {
        SSD1306_SET_PAGE_ADDR, // page addr 0
        0x02, // col start at 2 for SH1106
        0xe0, // RW-Start
    };

    if (!config::HAS_SH1106) {
        cmds[1] = SSD1306_NOP;
    }

    uint8_t cmds_end[] = {
        0xee, // RW-End
    };

    for (uint i = 0; i < 8; i++) {
        SSD1306_send_cmd_list(cmds, count_of(cmds));
        SSD1306_send_page(&frameBuffer[i * SSD1306_WIDTH]);
        SSD1306_send_cmd_list(cmds_end, count_of(cmds_end));
        cmds[0] += 1;
    }
}

RET_TYPE init(const int scl, const int sda) {
    puts("initting display!");

    i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);

    SSD1306_init();

    memset(frameBuffer, 0, SSD1306_BUF_LEN);

    render();

    return RET_SUCCESS;
}

void putpixel(int x, int y, bool b) {
    // buf[];
}

void print(int x, int y, const char *str) { WriteString(x, y, str); }
void printCentered(int y, const char *str) {
    int strLen = strlen(str);
    // 8 is the width of a single character
    int pad = (SSD1306_WIDTH / 8 - strLen) / 2;

    if (pad < 0)
        // text doesn't fit
        pad = 0;

    WriteString(pad * 8, y, str);
}

void printChar(int x, int y, const char c) { WriteChar(x, y, c); }

void printCustom(int16_t x, int16_t y, const uint8_t *custom,
                 unsigned int customLen) {
    int fb_idx = y * 128 + x;

    for (int i = 0; i < customLen; i++) {
        frameBuffer[fb_idx++] = custom[i];
    }
}

void display() {
    render();
    memset(frameBuffer, 0, SSD1306_BUF_LEN);
}

} // namespace display
