#! /bin/bash
pgrep -f "adb logcat"|xargs -i kill -9 {}
adb logcat -G 200M
adb logcat -c
for pid in $(adb shell pgrep -f com.android.camera)
do
    adb logcat --pid=$pid > $1_$pid.log &
    glogg $1_$pid.log
done

