#!/bin/bash
#repoUrl="git@git.n.xiaomi.com:MiuiCamera/miuicameratool.git"
#repoUrl="/pc2/work/test/.git/"

echo "repo url:>>>>"$1
echo "sync group:>>>>"$2
repoUrl=$1
group=$2
#repoName = miuicameratool
repoName=`basename $repoUrl .git`
echo $repoName
#repoName="test"

function uploadToRemote() {
#当前工作路径：~/.glogg/miuicameratool
    cd ~/.glogg/$repoName
    if [ ! -d glogg ]; then
        mkdir -p glogg
    fi
#配置文件copy到miuicameratool/glogg下
    cp ~/.config/glogg/sharedFilterSet.ini .

#删除“”
    sed -i "s/\([^\]\)\"/\1/g" sharedFilterSet.ini
#获取指定group($2)的filter数据
    grep "sharedfilter\\\[0-9]\+\\\filterItem=$group#" sharedFilterSet.ini > filters.txt
#For debug
#    cat filters.txt|while read line
#    do
#        echo $line
#    done

#删除前缀“sharedfilter\1\filterItem=”
    sed -i "s/sharedfilter\\\[0-9]\+\\\filterItem=[\"]*//g" filters.txt
#删除空行
    sed -i '/^$/d' filters.txt
#排序
    sort filters.txt | sed '$!N; /^\(.*\)\n\1$/!P; D' > $group.txt
    rm sharedFilterSet.ini


    if [ ! -d glogg ]; then
        mkdir -p glogg
        mv $group.txt glogg/
    else
        mv $group.txt glogg/
        #获取git diff 结果，进行预处理（删除+号、双引号）
        difftime=`date "+%Y-%m-%d_%H-%M-%S"`
        git diff|grep "^+[^+]"  > $difftime.diff
        sed -i "s/^+//g" $difftime.diff
        sed -i "s/\([^\]\)\"/\1/g" $difftime.diff
        git stash save
        git pull --rebase
        #$group.txt存在时，将diff文件追加到$group.txt中
        if [ -s glogg/$group.txt ]
        then
            rm filters.txt
            cat $difftime.diff >> glogg/$group.txt
        else
        #否则直接copy新建$group.txt
            mv filters.txt  glogg/$group.txt
        fi

        if [ ! -s $difftime.diff ]
        then
           rm $difftime.diff
        fi
        #排序
        sort glogg/$group.txt| sed '$!N; /^\(.*\)\n\1$/!P; D' > tmp.txt
        mv tmp.txt glogg/$group.txt

        ##重新恢复qsetting结构，为每行行首、行尾插入指定字符。NR为行号
        #awk '{print "sharedfilter\\" NR "\\filterItem=\"" $s "\""}' glogg/$group.txt > $group.txt
        #echo "[SharedFilterSet]" > tmp.txt
        ##行首加入[SharedFilterSet]
        #cat $group.txt >> tmp.txt
        ##行尾加入size和version结构
        #count=$(cat glogg.txt|wc -l)
        #echo "sharedfilter\\size=$count" >> tmp.txt
        #echo "version=1" >> tmp.txt
        ##重置本地配置文件
        #mv tmp.txt ~/.config/glogg/$group.ini
        #rm $group.txt
    fi
    #git stash pop
    git add glogg/$group.txt
    git add -u
    git commit -s -m "update filters of $group group."
    branch=$(git rev-parse --abbrev-ref HEAD)
    git push origin -u HEAD:$branch
    git clean -fd
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

if [ ! -n "$group" ];then
    echo "not input the group to sync!"
    echo "only update the miuicameratool project."
    cd ~/.glogg/$repoName
    git pull
else
    uploadToRemote
fi
