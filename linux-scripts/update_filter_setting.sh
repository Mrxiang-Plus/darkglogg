#!/bin/bash
#repoUrl="git@git.n.xiaomi.com:MiuiCamera/miuicameratool.git"
#repoUrl="/pc2/work/test/.git/"

echo "repo url:>>>>"$1
echo "sync group:>>>>"$2
repoUrl=$1
group=$2
endFlag="end"
#repoName = miuicameratool
repoName=`basename $repoUrl .git`
echo $repoName
#repoName="test"

function updateSetting() {
#当前工作路径：~/.glogg/miuicameratool
    cd ~/.glogg/$repoName
    if [ ! -d glogg ]; then
        mkdir -p glogg
    fi
    cd glogg/

    if [ -s $group.txt ]; then
        cat $group.txt >> temp.txt
        echo "add filters of $group group"
    else
        echo "$group.txt does not exist"
    fi

    if [ "$group" = "$endFlag" ];then
        echo "download end"
        echo "start recover ~/.config/glogg/sharedFilterSet.ini"
        #获取local filter配置
        cp ~/.config/glogg/sharedFilterSet.ini ./
        sed -i "s/\([^\]\)\"/\1/g" sharedFilterSet.ini
        grep "sharedfilter\\\[0-9]\+\\\filterItem=" sharedFilterSet.ini > localSetting.txt
        sed -i "s/sharedfilter\\\[0-9]\+\\\filterItem=[\"]*//g" localSetting.txt
        sed -i '/^$/d' filters.txt
        rm sharedFilterSet.ini
        cat localSetting.txt >> temp.txt
        sort -u temp.txt > uniq.txt


        #重新恢复qsetting结构，为每行行首、行尾插入指定字符。NR为行号
        awk '{print "sharedfilter\\" NR "\\filterItem=\"" $s "\""}' uniq.txt > temp1.txt
        #行首加入[SharedFilterSet]
        echo "[SharedFilterSet]" > sharedFilterSet.txt
        cat temp1.txt >> sharedFilterSet.txt
        #行尾加入size和version结构
        count=$(cat temp1.txt|wc -l)
        echo "sharedfilter\\size=$count" >> sharedFilterSet.txt
        echo "version=1" >> sharedFilterSet.txt
        #重置本地配置文件
        mv sharedFilterSet.txt ~/.config/glogg/sharedFilterSet.ini
        #clean invaild files
        rm temp.txt
        rm uniq.txt
        rm temp1.txt
        rm shareFilterSet.txt
        git clean -fd
    fi
}

#判断本地是否已包含miuicameratool项目
if [  ! -d ~/.glogg/$repoName ]; then
    echo "Error: Directory does not exists."
    cd ~/.glogg/
    git clone $repoUrl
#判断miuicameratool下是否包含glogg文件夹，若无则手动创建
    if [ ! -d $repoName/glogg ]; then
        mkdir -p $repoName/glogg
    fi
fi
updateSetting
