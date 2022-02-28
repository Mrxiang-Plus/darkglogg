#!/bin/bash
sudo killall glogg
sudo apt install -y glogg
sudo cp /usr/bin/glogg /usr/bin/gloggo
sudo cp glogg /usr/bin/glogg
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg/
cp linux-scripts/*.jar ~/.glogg/
cp linux-scripts/*.txt ~/.glogg/

if [ -f ~/.config/glogg/glogg.ini ]; then
    currentTime=`date "+%Y-%m-%d_%H-%M"`
    mv ~/.config/glogg/glogg.ini ~/.config/glogg/glogg_old_$currentTime.ini
    echo -e "您以前的配置文件是: ~/.config/glogg/glogg_old_$currentTime.ini\n需要恢复请执行:\nmv ~/.config/glogg/glogg2_old_$currentTime.conf ~/.config/glogg/glogg2.conf" >>  linux-scripts/readme.txt
else
    if [ -f ~/.config/glogg/glogg2.conf ]; then
        cp ~/.config/glogg/glogg2.conf ~/.config/glogg/glogg.ini
    else
        cp linux-scripts/glogg.ini ~/.config/glogg/glogg.ini
    fi
fi
glogg linux-scripts/readme.txt
