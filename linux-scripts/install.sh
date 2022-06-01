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
    cp ~/.config/glogg/glogg.ini ~/.config/glogg/glogg_old_$currentTime.ini
    echo -e "您以前的配置文件是: ~/.config/glogg/glogg_old_$currentTime.ini\n如原始配置丢失，请执行:\nmv ~/.config/glogg/glogg2_old_$currentTime.conf ~/.config/glogg/glogg2.conf 恢复" >>  linux-scripts/readme.txt
fi
glogg linux-scripts/readme.txt
