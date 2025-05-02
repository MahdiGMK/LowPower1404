#!/bin/bash
initialAddr=`./extractor.sh $1 $2`
if [ $? -eq 0 ]; then
  ./meet $initialAddr $1
else
  echo "Error in extracting the address of $2!"
fi
