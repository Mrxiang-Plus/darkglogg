#! /bin/bash
pgrep -f "adb logcat"|xargs -i kill -9 {}
adb logcat -G 200M
adb logcat -c
adb logcat > $1 &
glogg $1

