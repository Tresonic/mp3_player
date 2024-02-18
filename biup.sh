#!/bin/bash

mkdir -p build
cmake --build build -- -j8 && picotool load -x build/src/mp3_player.uf2 -f