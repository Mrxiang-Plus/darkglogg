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

if [ -f ~/.config/glogg/glogg2.conf ]; then
    currentTime=`date "+%Y-%m-%d_%H-%M"`
    mv ~/.config/glogg/glogg2.conf ~/.config/glogg/glogg2_old_$currentTime.conf
    echo -e "您以前的配置文件是: ~/.config/glogg/glogg2_old_$currentTime.conf\n需要恢复请执行:\nmv ~/.config/glogg/glogg2_old_$currentTime.conf ~/.config/glogg/glogg2.conf" >>  linux-scripts/readme.txt
fi
cp linux-scripts/glogg.conf ~/.config/glogg/glogg2.conf
glogg linux-scripts/readme.txt
