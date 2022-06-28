#! /bin/bash
unzipPath=$1
mode=$2

if [ "$mode" == "device" ]; then
    deviceId=$3
    pgrep -f "adb -s $deviceId logcat"|xargs -i kill -9 {}
    adb -s $deviceId logcat -G 100M
    adb -s $deviceId logcat -c
    echo "device=$deviceId and unspecified process"
    name=$1/$deviceId.log
    echo "========start logcat========" > $name
    adb -s $deviceId logcat >> $name &
    #sleep 1
    glogg $name
elif [ "$mode" == "device_pid" ]; then
    process=$3
    deviceId=$4
    echo "device=$deviceId and process=$process"
    for pid in $(adb -s $deviceId shell pgrep -f $process)
    do
        processName=`adb -s $deviceId shell ps -p $pid -o name=| tr -d '[:space:]'`
        if [ ! -z $processName ]; then
            name="${1}/${processName}_${deviceId}.log"
            echo "========start logcat========" > $name
            pgrep -f "adb -s $deviceId logcat"|xargs -i kill -9 {}
            adb -s $deviceId logcat -G 100M
            adb -s $deviceId logcat -c
            adb -s $deviceId logcat --pid=$pid >> $name &
            #sleep 1
            glogg $name
        fi
     done
else
    echo "single device and unspecified process"
    pgrep -f "adb logcat"|xargs -i kill -9 {}
    adb logcat -G 100M
    adb logcat -c
    adb logcat > $1/tmp.log &
    sleep 1
    glogg $1/tmp.log
fi
