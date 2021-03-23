#!/bin/sh
if  ! command -v bcompare &> /dev/null
then
    if   command -v meld &> /dev/null
    then
        meld $1 $2
    fi
else
    bcompare $1 $2
fi
