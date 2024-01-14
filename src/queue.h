#pragma once

void create_queue();

unsigned int get_queue_index();

bool set_queue_index(unsigned int index);

bool prev_queue_index();

bool next_queue_index(bool end_of_song = false);

void add_to_queue_at(const char *str, unsigned int index);

void add_to_queue_end(const char *str);

void add_to_queue_next(const char *str);

const char *get_queue_at(unsigned int index);

const char *get_cur_queue();

void shuffle_queue();

void clear_queue_not_index(unsigned int index);

void clear_queue();

void destroy_queue();
