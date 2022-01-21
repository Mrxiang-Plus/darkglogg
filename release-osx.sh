#!/bin/bash

# Build glogg for OSX and make a DMG installer
# (uses https://github.com/LinusU/node-appdmg)
#
# brew install node
# npm install -g appdmg
#
# QTDIR is built -static

QTDIR=/usr/local/Qt-5.9.5
BOOSTDIR=$HOME/work/boost_1_73_0
VERSION=$(date '+%Y_%m_%d')

#./Library/Preferences/com.glogg.glogg_pattern.plist

make clean
if [ ! -d "$BOOSTDIR" ]; then
    echo $BOOSTDIR not found.
    exit 1
elif [ -z "$VERSION" ]; then
    echo Please specify a version to build: VERSION=1.2.3 $0
    exit 1
else
    $QTDIR/bin/qmake glogg.pro LIBS+="-dead_strip" CONFIG+="release no-dbus version_checker" BOOST_PATH=$BOOSTDIR VERSION="$VERSION"
fi
make -j8
dsymutil release/glogg.app/Contents/MacOS/glogg
mv release/glogg.app/Contents/MacOS/glogg.dSYM release/glogg-$VERSION.dSYM

sed -e "s/\"glogg\"/\"glogg_$VERSION\"/" osx_installer.json >osx_${VERSION}_installer.json
rm glogg_${VERSION}_installer.dmg
appdmg osx_${VERSION}_installer.json glogg_${VERSION}_installer.dmg
rm osx_${VERSION}_installer.json

rm -rf package
mkdir package
cp -rf linux-scripts package/
cp -f package/linux-scripts/install-macos.sh package/
cp -f glogg_${VERSION}_installer.dmg package/
tar -czvf glogg_macos_$VERSION.tar.gz package
