#!/bin/bash
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg
if [ ! -f ~/.config/glogg/glogg.conf ]; then 
    cp linux-scripts/glogg.conf ~/.config/glogg/glogg.conf
fi
./glogg.sh linux-scripts/readme.txt

