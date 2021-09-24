#!/bin/bash
echo $1
echo " " >> $1
grep -o "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[^ ]*" $1 > time.txt
timeSum=0
index=0
while read d1_2; do
    read d2_2
    if [ ! -z $d2_2  ]; then
	  secdiff=`bc -l <<< "$(date -d $d2_2 +%s) - $(date -d $d1_2 +%s)"`
	  nanosecdiff=`bc -l <<< "$(date -d $d2_2 +%N) - $(date -d $d1_2 +%N)"`
	  printf "%s - %s = %d ms\n" $d2_2 $d1_2 $((
	    (secdiff * 1000) + (nanosecdiff / 1000000)
	  )) >> $1
	 echo $((
	    (secdiff * 1000) + (nanosecdiff / 1000000)
	  ))"ms"
	    timeSum=$((
	    (secdiff * 1000) + (nanosecdiff / 1000000) + $timeSum)) 

	index=$(($index+1))
	echo $index
    fi 
done < time.txt
echo "$index times, avg: `bc -l <<< $timeSum/$index` ms" >> $1
glogg $1
