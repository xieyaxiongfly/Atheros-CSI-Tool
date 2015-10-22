#!/bin/bash

make -j16
make modules
sudo make modules_install
Sudo make install
