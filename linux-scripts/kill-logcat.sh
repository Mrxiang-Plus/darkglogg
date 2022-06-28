#! /bin/sh
deviceId=$1
if [ -z $deviceId ];then
    pgrep -f "adb.*logcat"|xargs -i kill -9 {}
else
    pgrep -f "adb -s $deviceId logcat"|xargs -i kill -9 {}
fi

pgrep -f "cputools"|xargs -i kill -9 {}
