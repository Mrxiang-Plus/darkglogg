#!/bin/bash
PROGUARD_HOME=`dirname "$0"`/
cd $2

filename=$(dirname "$3")
echo "------"
echo $filename
grepPath="$4/[^/]*"
filename=$(dirname "$3"|grep -o $grepPath)
echo ">>>>"
echo $filename
echo ">>>>"
if [ -d "$filename" ];then
    cd $filename
    mappingFile=`find . -iname "mapping_*.txt"`
    echo ">>>>>>>>>>>"
    echo $mappingFile
    echo "<<<<<<<<<<"
    if [ -z "$mappingFile" ];then 
	versionCodeArray=(`find . -type f \( -name '*.log' -o -name '*.txt' \) -print0|xargs -0 -I % sed -e '/Package \[com\.android\.camera\] (/,/versionName=/!d' %|grep versionName=|grep -o "[0-9\.]*"|awk '{print $1}'`)
	versionCode="V"${versionCodeArray[0]}
	refCode=$versionCode".*^"
	versionMatch=$versionCode"[^^]*"
	echo "$refCode"
	versionFullCode=`git ls-remote --tags git@git.n.xiaomi.com:MiuiCamera/cameramapping.git|grep $refCode|grep -o $versionMatch`
	echo ">>>>>"$versionFullCode"<<<<<<"
	git archive --remote=git@git.n.xiaomi.com:MiuiCamera/cameramapping.git $versionFullCode mapping.txt|tar xvf -
	mv mapping.txt mapping_$versionFullCode.txt
	java -jar $PROGUARD_HOME/retrace.jar -verbose mapping_$versionFullCode.txt $1 > $2/out.txt
    else
	java -jar $PROGUARD_HOME/retrace.jar -verbose $mappingFile $1 > $2/out.txt
    fi
glogg $2/out.txt
fi
