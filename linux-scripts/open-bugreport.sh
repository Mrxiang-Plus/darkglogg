#!/bin/bash
filename=$(basename $1 .zip)

unzip -o $1 -d $2/$filename 
cd $2/$filename
find . -type f -name '*.zip' -print0|xargs -I % unzip -o %
find . -type f -name 'bugreport*.txt' |xargs -I % glogg %
