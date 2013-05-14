#!/bin/bash

if [ $1 ]
then
  LINES=$(wc -l $1)
  echo "LINES: $LINES"
<<<<<<< HEAD
  ./monCSRF -v $1
else
  echo " =/"
=======
  time ./bin/parser.rb $1 -b
else
  echo "Log File not found or not specified."
>>>>>>> b265d9806d0c7629e7eff6ef2fc8d8c8df5f52a6
fi
