#! /bin/bash
pgrep -f "adb logcat"|xargs -i kill -9 {}
adb logcat -G 100M
adb logcat -c
unzipPath=$1
mode=$2

if [ "$mode" == "device" ]; then
    deviceId=$3
    echo "device=$deviceId and unspecified process"
    adb -s $deviceId logcat > $1/tmp.log &
    glogg $1/tmp.log
elif [ "$mode" == "pid" ]; then
    process=$3
    echo "single device and process=$process"
    for pid in $(adb shell pgrep -f $process)
    do
        processName=`adb shell ps -p $pid -o name=| tr -d '[:space:]'`
        if [ ! -z $processName ]; then
            name=$1/$processName.log
            adb logcat --pid=$pid > $name &
            glogg $name
        fi
     done
elif [ "$mode" == "device_pid" ]; then
    process=$3
    deviceId=$4
    echo "device=$deviceId and process=$process"
    for pid in $(adb -s $deviceId shell pgrep -f $process)
    do
        processName=`adb -s $deviceId shell ps -p $pid -o name=| tr -d '[:space:]'`
        if [ ! -z $processName ]; then
            name=$1/$processName.log
            adb -s $deviceId logcat --pid=$pid > $name &
            glogg $name
        fi
     done
else
    echo "single device and unspecified process"
    adb logcat > $1/tmp.log &
    glogg $1/tmp.log
fi
