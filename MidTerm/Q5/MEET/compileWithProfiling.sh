#!/bin/bash
make clean
make config-arm
PROFILE_OPTIONS="-fprofile-generate" make && sh validateInstall.sh && make clean && make config-arm && PROFILE_OPTIONS="-fprofile-use" make
