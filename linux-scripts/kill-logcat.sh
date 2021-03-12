#! /bin/sh
pgrep -f "adb logcat"|xargs -i kill -9 {}
