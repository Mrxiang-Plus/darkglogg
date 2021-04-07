#!/bin/bash
PROGUARD_HOME=`dirname "$0"`/
cd $2
versionCode="V"`sed -e '/Package \[com\.android\.camera\] (/,/versionName=/!d' $3|grep  versionName=|awk -F\= '{print $2}'`
echo ">>>>>"$versionCode"<<<<<<"
git archive --remote=git@git.n.xiaomi.com:MiuiCamera/cameramapping.git $versionCode mapping.txt|tar xvf -
java -jar $PROGUARD_HOME/retrace.jar -verbose mapping.txt $1 > $2/out.txt
glogg $2/out.txt
