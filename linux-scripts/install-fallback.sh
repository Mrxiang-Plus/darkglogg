#!/bin/bash
sudo apt-get install glogg
if [ ! -d ~/.glogg ]; then
    mkdir ~/.glogg
fi
cp linux-scripts/*.sh ~/.glogg
if [ ! -f ~/.config/glogg/glogg.conf ]; then 
    cp linux-scripts/glogg.conf ~/.config/glogg/glogg.conf
fi

gloggPath=$(command -v glogg)
if [ ! -z "$gloggPath" ]
then
    sudo mv $gloggPath /usr/bin/glogg_old
else
    sudo cp hicolor/16x16/glogg.png /share/icons/hicolor/16x16/apps/glogg.png
    sudo cp hicolor/32x32/glogg.png /share/icons/hicolor/32x32/apps/glogg.png
    sudo cp hicolor/scalable/glogg.svg /share/icons/hicolor/scalable/apps/glogg.svg
    sudo cp glogg.desktop /share/applications/glogg.desktop
fi
echo "$PWD/glogg \$*" >> glogg.sh
if [ ! -z "$gloggPath" ]
then
    sudo ln -s $PWD/glogg.sh $gloggPath
else 
    sudo ln -s $PWD/glogg.sh /usr/bin/glogg fi
glogg linux-scripts/readme.txt

