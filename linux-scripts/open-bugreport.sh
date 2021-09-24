#!/bin/bash
function ex () {
    if [ -f "$1" ]
        then
            case "$1" in
            (*.tar.bz2) tar xvjf "$1" ;;
            (*.tar.gz) tar xvzf "$1" ;;
            (*.bz2) bunzip2 "$1" ;;
            (*.rar) unrar x -o+ "$1" ;;
            (*.gz) gunzip "$1" ;;
            (*.tar) tar xvf "$1" ;;
            (*.tbz2) tar xvjf "$1" ;;
            (*.tgz) tar xvzf "$1" ;;
            (*.zip) unzip -o "$1" ;;
            (*.Z) uncompress "$1" ;;
            (*.7z) 7z x "$1" ;;
            (*) echo "'$1' cannot be extracted via extract" ;;
    esac
    else
        echo "'$1' is not a valid file"
            fi
}

filename=$(basename "$1")
#name=$(echo "$filename" | cut -f 1 -d '.')
name=${filename%.*}

echo "$name"
rm -rf "$2/$name"
mkdir -p "$2/$name"
cd "$2/$name"
ex "$1"
rm -rf "$1"
for value in {1..10}
do
    var=`find . -type f -name 'bugreport*.txt'`
    if [ -z "$var" ];then
        mapfile -d $'\0' array < <(find . -name "*.zip" -print0)
find . -type f -name '*.zip' -print0|xargs -0 -I % unzip -o %
        for i in "${!array[@]}";
        do
            echo "delete ${array[$i]}"
            rm "${array[$i]}"
        done
    else
        break
    fi
done

find . -type f -name 'bugreport*.txt' -print0|xargs -0 -I % glogg %
#find . -type f -name 'bugreport*.txt' -print0|xargs -0 -I % sed -i 's/\(^[0-9]*-[0-9]* [0-9:]*[^\.]*\.[0-9]*\) [^ ]*/\1/g' %
#find . -type f -name 'bugreport*.txt' -print0|xargs -0 -I % touch %
find . -type f -name '*.mp4' -print0|xargs -0 -I % gnome-open %
find . -type f -iname "*.png" -print0|xargs -0 feh -t -Sfilename -E 479 -y 479 -W 960 &
var=`find . -type f -name 'bugreport*.txt'`
if [ -z "$var" ];then
    find . -type f -name 'test_*Times*[0-9].log' -print0|xargs -0 -I % glogg %
#    find . -type f -name 'test_*Times*[0-9].log' -print0|xargs -0 -I % sed -i 's/\(^[0-9]*-[0-9]* [0-9:]*[^\.]*\.[0-9]*\) [^ ]*/\1/g' %
#    find . -type f -name 'test_*Times*[0-9].log' -print0|xargs -0 -I % touch %
fi


echo "find and merge logcat"
logcat_name="$name"_logcat.txt
count=$(find . -type f -name 'logcatlog.txt'|wc -l)
count=$[$(find . -type f -name 'logcatlog.txt.*'|wc -l)+$count]
echo $count
if [[ $count -gt 0 ]];then
    touch "$logcat_name"
    find . -type f -name 'logcatlog.txt.*' -print0|sort -z -r|xargs -0 -I % dd if=% bs=4k of="$logcat_name" oflag=append conv=notrunc
    find . -type f -name 'logcatlog.txt'|xargs -I % dd if=% bs=4k of="$logcat_name" oflag=append conv=notrunc
    var=`find . -type f -name 'bugreport*.txt'`
    if [ -z "$var" ];then
        glogg  "$logcat_name"
    fi
fi

echo "find and merge camera app logcat"
camera_log_name="$name"_cam_log.txt
count=$(find . -type f -name 'com.android.camera.log'|wc -l)
count=$[$(find . -type f -name 'com.android.camera.log.*'|wc -l)+$count]
if [[ $count -gt 0 ]];then
    find . -type f -name 'com.android.camera.log.*' -print0|sort -z -r|xargs -0 -I % dd if=% bs=4k of="$camera_log_name" oflag=append conv=notrunc
    find . -type f -name 'com.android.camera.log'|xargs -I % dd if=% bs=4k of="$camera_log_name" oflag=append conv=notrunc
    sed -i 's/^[^-]*-\(.*\),\([0-9]*\) *- \[\([A-Z]\)[^[]*\[\([^-]*\)-\([^]]*\)\] -/\1.\2 \4 \5 \3/g' "$camera_log_name"
    awk '{$3=sprintf("%5s %5s", $3, $4);$4=""}1' "$camera_log_name" > tmp.txt
    sed -i 's/  \([A-Z]\) / \1 /g' tmp.txt
    mv tmp.txt "$camera_log_name"
    touch "$camera_log_name"
fi
