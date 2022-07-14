
deviceId=$1
apkPath=$2
export ANDROID_SERIAL=$deviceId
adb root
adb remount
adb install -r -d $apkPath | grep "Success" > output.txt
if [ ! -s output.txt ];then
    rm MiuiCamera.apk
    cp $apkPath MiuiCamera.apk
    adb push MiuiCamera.apk /system/priv-app/MiuiCamera/MiuiCamera.apk
    adb reboot
fi
rm output.txt

