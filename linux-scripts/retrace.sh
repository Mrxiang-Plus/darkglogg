#!/bin/bash
PROGUARD_HOME=`dirname "$0"`/
cd $2
versionCode="V"`sed -e '/Package \[com\.android\.camera\] (/,/versionName=/!d' $3|grep  versionName=|grep -o "[0-9\.]*"`
#adbVersionCode=`adb shell dumpsys package com.android.camera| grep -A18 "Package \[com.android.camera\]"|grep versionName|sed 's/versionName=/V/'`
echo ">>>>>"$versionCode"<<<<<<"
refCode=$versionCode".*^"
versionMatch=$versionCode"[^^]*"
echo "$refCode"
versionFullCode=`git ls-remote --tags git@git.n.xiaomi.com:MiuiCamera/cameramapping.git|grep $refCode|grep -o $versionMatch`
echo ">>>>>"$versionFullCode"<<<<<<"
git archive --remote=git@git.n.xiaomi.com:MiuiCamera/cameramapping.git $versionFullCode mapping.txt|tar xvf -
mv mapping.txt mapping_$versionFullCode.txt
java -jar $PROGUARD_HOME/retrace.jar -verbose mapping_$versionFullCode.txt $1 > $2/out.txt
glogg $2/out.txt
