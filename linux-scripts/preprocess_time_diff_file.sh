#!/bin/bash
echo " " >> $1
#echo "unit:>>>" $2
#echo "StringList" >> $3
dir=${1%/*}
open="open"
deal="deal"
cd $dir
if [ $2 == $deal ]
then
    if [ -s time.log ]
    then
        rm time.log
        echo "rm"
    fi
    grep -o "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[^ ]*" $1 > time.log
fi

if [ $2 == $open ]
then
    glogg $1
fi

echo "done"
#
#cat time.txt .>> time.log

#i=0
#until [ $i -lt $2 ]
#do
#    echo "i:>>>"  $i
#    awk "NR%$2 == 0" time.txt > temp_$i.txt
#done
#timeSum=0
#index=0
#
##echo "$index times, avg: `bc -l <<< $timeSum/$index` ms" >> $1
#touch $1
##glogg $1
#glogg time.log
#glogg temp_2.txt
