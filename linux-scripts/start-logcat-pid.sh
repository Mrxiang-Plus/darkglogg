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
        name=$1/`adb shell ps -p $pid -o name=| tr -d '[:space:]'`_$pid.log
        adb logcat --pid=$pid > $name &
        glogg $name
    done
fi

