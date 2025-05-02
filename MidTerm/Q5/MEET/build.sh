#!/bin/bash
arm-none-eabi-gcc -v 2> /dev/null 1> /dev/null
if [ $? == 0 ]; then
  if [[ $# -le 1 ]]; then
    echo "Usage: sh build.sh pathToSource.c pathToOutputFile"
    exit 1
  else
    # arm-none-eabi-gcc --specs=nosys.specs -static -mcpu=arm7tdmi -marm -Wl,--fix-v4bx -msoft-float -Tat91.ld -nostartfiles vectors.s crti.o $*
    arm-none-eabi-gcc --specs=nosys.specs -static -mcpu=arm7tdmi -marm -Wl,--fix-v4bx -msoft-float -Tat91.ld -nostartfiles vectors.s $*
  fi
else
  echo "arm-none-eabi-gcc not found!"
  exit 2
fi
