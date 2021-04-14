#!/bin/bash
function ex () {
    if [ -f $1 ]
        then
            case $1 in
            (*.tar.bz2) tar xvjf $1 ;;
            (*.tar.gz) tar xvzf $1 ;;
            (*.bz2) bunzip2 $1 ;;
            (*.rar) unrar -o+ x $1 ;;
            (*.gz) gunzip $1 ;;
            (*.tar) tar xvf $1 ;;
            (*.tbz2) tar xvjf $1 ;;
            (*.tgz) tar xvzf $1 ;;
            (*.zip) unzip -o $1 ;;
            (*.Z) uncompress $1 ;;
            (*.7z) 7z x $1 ;;
            (*) echo "'$1' cannot be extracted via extract" ;;
    esac
    else
        echo "'$1' is not a valid file"
            fi
}

filename=$(basename $1 .zip)
mkdir -p $2/$filename 
cd $2/$filename
ex $1
find . -type f -name '*.zip' -print0|xargs -I % unzip -o %
find . -type f -name 'bugreport*.txt' |xargs -I % glogg %
find . -type f -name 'test*.log' |xargs -I % glogg %
find . -type f -name 'main_log*' |xargs -I % glogg %

