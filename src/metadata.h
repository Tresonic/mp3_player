#pragma once

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int TITLE_LEN = 33;
const int ARTIST_LEN = 42;
const int ALBUM_LEN = 64;
const int YEAR_LEN = 5;
const int GENRE_LEN = 64;

// Structure to store ID3 tag information
// multiple artists/albums/genre are sepparated by a ';'
struct ID3Tag {
    char title[TITLE_LEN + 1];
    char artist[ARTIST_LEN + 1];
    char album[ALBUM_LEN + 1];
    char year[YEAR_LEN + 1];
    char genre[GENRE_LEN + 1];
};

// Function to parse ID3 tags from an MP3 buffer
void parse_mp3_metadata(const char *mp3_buffer, size_t tag_size,
                        struct ID3Tag *metadata) {
    // Clear old metadata
    metadata->title[0] = '\0';
    metadata->artist[0] = '\0';
    metadata->album[0] = '\0';
    metadata->year[0] = '\0';
    metadata->genre[0] = '\0';

    // Check if the buffer is large enough to contain the ID3 header
    if (tag_size < 10 || strncmp(mp3_buffer, "ID3", 3) != 0) {
        printf("ERROR: No valid ID3 header found in the buffer.\n");
        return;
    }

    // Parse the tag version
    int version = (int)mp3_buffer[3];
    // Adjust the structure based on the ID3 version
    if (version == 3) {
        // ID3v2.3
        mp3_buffer += 10;
    } else if (version == 4) {
        // ID3v2.4
        mp3_buffer += 10;
    } else {
        printf("ERROR: No valid ID3 header found in the buffer.\n");
        return;
    }

    // Iterate over frames in the tag content
    while (tag_size > 0) {
        // Check if there's enough space for the frame header
        if (tag_size < 10) {
            printf("ERROR: Invalid frame header in ID3 tag.\n");
            return;
        }

        // Read frame ID
        char frame_id[5];
        strncpy(frame_id, mp3_buffer, 4);
        frame_id[4] = '\0';

        // Read frame size
        int frame_size = (mp3_buffer[4] << 24) | (mp3_buffer[5] << 16) |
                         (mp3_buffer[6] << 8) | mp3_buffer[7];

        // Move to the beginning of the frame content
        mp3_buffer += 10;

        // Check if there's enough space for the frame content
        if (tag_size < frame_size) {
            printf("ERROR: Invalid frame content in ID3 tag.\n");
            return;
        }

        mp3_buffer++;
        frame_size--;

        // Process frame content based on frame ID
        if (strcmp(frame_id, "TIT2") == 0 && frame_size <= TITLE_LEN) {
            strncpy(metadata->title, mp3_buffer, frame_size);
            metadata->title[frame_size] = '\0';
        } else if (strcmp(frame_id, "TPE1") == 0 && frame_size <= ARTIST_LEN) {
            strncpy(metadata->artist, mp3_buffer, frame_size);
            metadata->artist[frame_size] = '\0';
        } else if (strcmp(frame_id, "TALB") == 0 && frame_size <= ALBUM_LEN) {
            strncpy(metadata->album, mp3_buffer, frame_size);
            metadata->album[frame_size] = '\0';
        } else if ((strcmp(frame_id, "TYER") == 0 ||
                    strcmp(frame_id, "TDRL") == 0) &&
                   frame_size <= YEAR_LEN) {
            strncpy(metadata->year, mp3_buffer, frame_size);
            metadata->year[frame_size] = '\0';
        } else if (strcmp(frame_id, "TCON") == 0 && frame_size <= GENRE_LEN) {
            strncpy(metadata->genre, mp3_buffer, frame_size);
            metadata->genre[frame_size] = '\0';
        }
        // Move to the next frame
        mp3_buffer += frame_size;
        tag_size -= frame_size;
    }
}

void print_metadata(struct ID3Tag *metadata) {
    printf("Title: %s\n", metadata->title);
    printf("Artist: %s\n", metadata->artist);
    printf("Album: %s\n", metadata->album);
    printf("Year: %s\n", metadata->year);
    printf("Genre: %s\n", metadata->genre);
}
