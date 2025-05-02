#!/bin/bash
if [[ $# -ne 2 ]]; then
    echo "Usage: ./extractor.sh fullpathToBinary startupFunctionName"
    exit 1
else
  t=`readelf -s $1 | grep -e " $2$"`;
  t="${t#*: }";
  startAddress="${t%% *}";
  t=${t#* };
  size="${t%%F*}";
	  size="${size%%O*}";
  startAddress=`echo "$startAddress" | tr '[:lower:]' '[:upper:]'`;
  if [ "$startAddress" == "" ]; then
    exit 2
  else
    echo -n " -initial:pc 0x$startAddress ";
  fi
fi
