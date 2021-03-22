#!/bin/sh
sed "s/^[0-9\:\.\ -]\+[^A-Z]//g" $1 > a.txt
sed "s/^[0-9\:\.\ -]\+[^A-Z]//g" $2 > b.txt

meld a.txt b.txt
#bcompare a.txt b.txt
