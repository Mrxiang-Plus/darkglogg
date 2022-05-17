adb shell getprop | grep ro.product.marketname > device_info.txt
adb shell getprop | grep ro.product.device >> device_info.txt
adb shell getprop | grep ro.product.mod_device >> device_info.txt
adb shell getprop | grep ro.boot.hwc >> device_info.txt
adb shell getprop | grep ro.miui.build.region >> device_info.txt
echo >> device_info.txt
echo "CPU INFO" >> device_info.txt
adb shell cat /proc/cpuinfo| grep Hardware >> device_info.txt
glogg device_info.txt
