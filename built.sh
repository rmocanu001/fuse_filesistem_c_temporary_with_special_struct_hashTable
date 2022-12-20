#!/bin/bash

if [ -z $CC ]
then 
	CC=cc
fi
$CC -Wall fuse-main.c -D_FILE_OFFSET_BITS=64 -l fuse -o fuse-main
	
