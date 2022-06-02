#! /bin/sh
pgrep -f "adb logcat"|xargs -i kill -9 {}
pgrep -f "cputools"|xargs -i kill -9 {}
