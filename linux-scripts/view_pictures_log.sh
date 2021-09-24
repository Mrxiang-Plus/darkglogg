#!/bin/bash
filename=$(dirname "$1")
echo $filename
grepPath=$2"/[^/]*"
filename=$(dirname "$1"|grep -o $grepPath)
echo $filename
if [ -d "$filename" ];then
    cd $filename
    var=`find . -type f \( -name '*.png' -o -name '*.jpg' \)`
    if [ ! -z "$var" ];then
        find . -type f \( -name '*.png' -o -name '*.jpg' \) -print0|xargs -0 feh -t -Sfilename -E 492 -y 479 -W 960 --scale-down &
    fi
fi
