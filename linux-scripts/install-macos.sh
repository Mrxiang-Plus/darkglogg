#!/bin/bash
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg/
cp linux-scripts/*.jar ~/.glogg/

if [ -f ~/.config/glogg/glogg.ini ]; then
    currentTime=`date "+%Y-%m-%d_%H-%M"`
    mv ~/.config/glogg/glogg.ini ~/.config/glogg/glogg_old_$currentTime.ini
    echo -e "您以前的配置文件是: ~/.config/glogg/glogg_old_$currentTime.ini\n需要恢复请执行:\nmv ~/.config/glogg/glogg2_old_$currentTime.conf ~/.config/glogg/glogg2.conf" >>  linux-scripts/readme.txt
fi
cp linux-scripts/glogg.ini ~/.config/glogg/glogg.ini
glogg linux-scripts/readme.txt
