#include "queue.h"

#include "pico/stdlib.h"
#include <algorithm>
#include <random>
#include <string>
#include <vector>

// potential problems:
// queue will automatically size but

unsigned int cur_index;
bool changing_index;
std::vector<std::string> *queue;
auto rng = std::default_random_engine{};

// only for testing purposes
void print_queue() {
    int i = 0;
    printf("QUEUE -----\n");
    while (get_queue_at(i)) {
        printf("%s\n", get_queue_at(i));
        i++;
    }
    printf("----- QUEUE\n");
}

void create_queue() {
    queue = new std::vector<std::string>;
    // reserve queue of at least 10 elements
    queue->reserve(20);
    cur_index = 0;
    changing_index = false;

    // setting the seed is only needed once. This will use the time since
    // startup. in short testing has been random enough for a shuffle
    // algorithm...
    rng.seed(int(time_us_64()));
}

unsigned int get_queue_index() { return cur_index; }

bool set_queue_index(unsigned int index) {
    // allow index above size to stop after last song
    if (index >= 0 && index <= queue->size()) {
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

bool next_queue_index(bool end_of_song) {
    if (end_of_song && changing_index) {
        changing_index = false;
        return false;
    } else {
        changing_index = true;
        return set_queue_index(cur_index + 1);
    }
}

void add_to_queue_at(const char *str, unsigned int index) {
    queue->insert(queue->begin() + index, str);
}

void add_to_queue_end(const char *str) { queue->push_back(str); }

void add_to_queue_next(const char *str) {
    queue->insert(queue->begin() + cur_index + 1, str);
}

const char *get_queue_at(unsigned int index) {
    if (index < 0 || index >= queue->size()) {
        return NULL;
    } else {
        return (*queue)[index].c_str();
    }
}

const char *get_cur_queue() { return get_queue_at(cur_index); }

void shuffle_queue() {
    // current element is now first element in queue
    std::iter_swap(queue->begin() + cur_index, queue->begin());
    cur_index = 0;

    // randomize rest of queue:
    std::shuffle(queue->begin() + 1, queue->end(), rng);
}

void clear_queue_not_index(unsigned int index) {
    for (int i = 0; i < queue->size(); i++) {
        if (i != index) {
            queue->erase(queue->begin() + i);
        }
    }
}

void clear_queue() { queue->clear(); }

void destroy_queue() {
    clear_queue();
    delete queue;
}
