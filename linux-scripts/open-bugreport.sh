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

mkdir -p "$2/$name"
cd "$2/$name"
ex "$1"
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
find . -type f -name '*.mp4' -print0|xargs -0 -I % gnome-open %


logcat_name="$name"_logcat.txt
count=$(find . -type f -name 'logcatlog.txt.*'|wc -l)
if [[ $count -gt 0 ]];then
    find . -type f -name 'logcatlog.txt.*'|sort -r|xargs cat > "$logcat_name"
    find . -type f -name 'logcatlog.txt'|xargs cat >> "$logcat_name"
    var=`find . -type f -name 'bugreport*.txt'`
    if [ -z "$var" ];then
        glogg  "$logcat_name"
    fi
fi

camera_log_name="$name"_cam_log.txt
count=$(find . -type f -name 'com.android.camera.log.*'|wc -l)
if [[ $count -gt 0 ]];then
    find . -type f -name 'com.android.camera.log.*'|sort -r|xargs cat > "$camera_log_name"
    find . -type f -name 'com.android.camera.log'|xargs cat >> "$camera_log_name"
    sed -i 's/^[^-]*-\(.*\),\([0-9]*\) *- \[\([A-Z]\)[^[]*\[\([^-]*\)-\([^]]*\)\] -/\1.\2 \4 \5 \3/g' "$1"
    awk '{$3=sprintf("%5s %5s", $3, $4);$4=""}1' "$camera_log_name" > tmp.txt
    sed -i 's/  \([A-Z]\) / \1 /g' tmp.txt
    mv tmp.txt "$camera_log_name"
    touch "$camera_log_name"
else
    find . -type f -name 'com.android.camera.log'|xargs -I % cp % "$camera_log_name"
    sed -i 's/^[^-]*-\(.*\),\([0-9]*\) *- \[\([A-Z]\)[^[]*\[\([^-]*\)-\([^]]*\)\] -/\1.\2 \4 \5 \3/g' "$1"
    awk '{$3=sprintf("%5s %5s", $3, $4);$4=""}1' "$camera_log_name" > tmp.txt
    sed -i 's/  \([A-Z]\) / \1 /g' tmp.txt
    mv tmp.txt "$camera_log_name"
    touch "$camera_log_name"
fi
