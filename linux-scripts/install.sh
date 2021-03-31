#!/bin/bash
sudo apt-get install glogg
sudo cp /usr/bin/glogg /usr/bin/gloggo
sudo cp glogg /usr/bin/glogg
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg

if [ ! -f ~/.config/glogg/glogg2.conf ]; then
    if [ -f ~/.config/glogg/glogg.conf ]; then
        cp  ~/.config/glogg/glogg.conf ~/.config/glogg/glogg2.conf
    else
        cp linux-scripts/glogg.conf ~/.config/glogg/glogg2.conf
    fi
fi
glogg linux-scripts/readme.txt

