#!/bin/bash

make clean
make all -j$(nproc)
make install
