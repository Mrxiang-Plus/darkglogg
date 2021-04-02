#! /bin/bash
pgrep -f "adb logcat"|xargs -i kill -9 {}
adb logcat -G 100M
adb logcat -c
if [ -z $2 ]; then
    adb logcat > $1/tmp.log &
    glogg $1/tmp.log
else
    for pid in $(adb shell pgrep -f $2)
    do
        processName=`adb shell ps -p $pid -o name=| tr -d '[:space:]'`
        if [ ! -z $processName ]; then
            name=$1/$processName.log
            adb logcat --pid=$pid > $name &
            glogg $name
        fi
    done
fi

