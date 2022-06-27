#! /bin/bash
deviceId=$2
if [-z $deviceId ]; then
    pgrep -f "adb logcat"|xargs -i kill -9 {}
    adb logcat -G 200M
    adb logcat -c
    adb logcat > $1/tmp.log &
else
    pgrep -f "adb -s $deviceId logcat"|xargs -i kill -9 {}
    adb -s $deviceId logcat -G 200M
    adb -s $deviceId logcat -c
    adb -s $deviceId logcat > $1/tmp.log &
fi
    glogg $1/tmp.log


