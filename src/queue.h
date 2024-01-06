#pragma once

#include "config.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max_path;
int max_queue;
int next_top_index;
int cur_index;
bool changing_index;
char **queue;

void create_queue() {
    next_top_index = 0;
    max_path = config::MAX_PATH_LEN;
    max_queue = config::MAX_QUEUE_LEN;
    cur_index = 0;
    changing_index = false;
    queue = (char **)malloc(max_queue * sizeof(char *));
}

uint get_queue_index() { return cur_index; }

bool set_queue_index(uint index) {
    // allow next_top_index to stop after last song
    if (index >= 0 && index <= next_top_index) {
        cur_index = index;
        return false;
    } else {
        return true;
    }
}

bool prev_queue_index() {
    changing_index = true;
    return set_queue_index(cur_index - 1);
}

bool next_queue_index(bool end_of_song = false) {
    if (end_of_song && changing_index) {
        changing_index = false;
        return false;
    } else {
        changing_index = true;
        return set_queue_index(cur_index + 1);
    }
}

bool add_to_queue_at(char *str, size_t str_len, int index) {
    if (str_len > max_path || next_top_index == max_queue) {
        // error
        return 1;
    } else {
        char *path = (char *)malloc((str_len + 1) * sizeof(char));
        strncpy(path, str, str_len + 1);

        for (int i = next_top_index++; i > index; i--) {
            queue[i] = queue[i - 1];
        }

        queue[index] = path;
        return 0;
    }
}

bool add_to_queue(char *str, size_t str_len) {
    return add_to_queue_at(str, str_len, next_top_index);
}

char *get_queue_at(int index) {
    if (index < 0 || index >= next_top_index) {
        return NULL;
    } else {
        return queue[index];
    }
}

char *get_cur_queue() { return get_queue_at(cur_index); }

void clear_queue_not_index(int index) {
    for (int i = 0; i < next_top_index; i++) {
        if (i == index) {
            queue[0] = queue[i];
        } else {
            free(queue[i]);
        }
    }
    next_top_index = 1;
}

void clear_queue() {
    for (int i = 0; i < next_top_index; i++) {
        free(queue[i]);
    }
    next_top_index = 0;
}

void destroy_queue() {
    clear_queue();
    free(queue);
}
