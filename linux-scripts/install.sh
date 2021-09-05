#!/bin/bash
sudo killall glogg
sudo apt-get install glogg
sudo cp /usr/bin/glogg /usr/bin/gloggo
sudo cp glogg /usr/bin/glogg
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg/
cp linux-scripts/*.jar ~/.glogg/

if [ ! -f ~/.config/glogg/glogg2.conf ]; then
    cp linux-scripts/glogg.conf ~/.config/glogg/glogg2.conf
fi
glogg linux-scripts/readme.txt
