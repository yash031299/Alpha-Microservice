#!/bin/bash
set -e
conan install . --output-folder=build --build=missing
meson setup build --wipe
meson compile -C build
