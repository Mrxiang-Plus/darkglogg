
mode=$1
deviceId=$2
deviceMode="device"
pidMode="pid"

function getDeviceList() {
    rm device.txt
    adb devices > temp.txt
    sed -i "1d" temp.txt
    sed "s/device//g" temp.txt > temp1.txt
    for line in $(cat temp1.txt)
    do
            echo -n $(adb -s $line shell getprop ro.product.name) >> device.txt
            echo -n "_" >> device.txt
            echo $line >> device.txt
    done
    rm temp.txt temp1.txt
}

function getProcesses() {
    if [ -z $2 ];then
        adb shell pm list packages -e >> temp2.txt
    else
        adb -s $2 shell pm list packages -e >> temp2.txt
    fi
    sed "s/package://g" temp2.txt >> process.txt
    rm temp2.txt
}

cd ~/.glogg
if [ "$mode" = "$deviceMode" ];then
    echo "get device list"
    getDeviceList
elif [ "$mode" = "$pidMode" ];then
    echo "get pid list"
    getProcesses
else
    echo "invaild mode"
fi
