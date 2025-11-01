#!/bin/bash

PROJECT=$1

read a <<< $(cat ${PROJECT}.lst | tail -1 | awk '{print $1}')
hex="${a%%[[:alpha:]]*}"
length=$(printf '%d' "0x$hex")

xxd -i -l ${length} rom1.bin > ${PROJECT}.h
