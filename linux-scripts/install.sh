#!/bin/bash
sudo apt-get install glogg
sudo cp glogg /usr/bin/glogg
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg
if [ ! -f ~/.config/glogg/glogg.conf ]; then 
    cp linux-scripts/glogg.conf ~/.config/glogg/glogg.conf
fi
glogg linux-scripts/readme.txt

