#!/bin/bash
if (( $# < 1 )); then
    echo "Usage: ./varLister.sh projectFolder"
else
    ctags -R -x --sort=yes --c-kinds=v --file-scope=no $* | awk '{ print $1}'
fi
