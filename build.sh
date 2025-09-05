#!/usr/bin/env bash

rm -rf build
meson setup build
meson compile -C build
