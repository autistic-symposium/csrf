#!/bin/bash

if [ $1 ]
then
  LINES=$(wc -l $1)
  echo "LINES: $LINES"
  time ./bin/parser.rb $1 -b
else
  echo "Log File not found or not specified."
fi
