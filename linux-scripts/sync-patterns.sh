#!/bin/bash
#repoUrl="git@git.n.xiaomi.com:MiuiCamera/miuicameratool.git"
#repoUrl="/pc2/work/repo/.git/"

echo "repo url:>>>>"$1
repoUrl=$1
repoName=`basename $repoUrl .git`
#repoName="repo"
function uploadToRemote() {
    cd ~/.glogg/$repoName
    if [ ! -d glogg ]; then
        mkdir -p glogg
    fi
    cp ~/.config/glogg/glogg_pattern.conf glogg/
    difftime=`date "+%Y-%m-%d_%H-%M-%S"`
    git diff|grep "^+searchPattern\\\[0-9\]"  > $difftime.diff
    sed -i "s/^+//g" $difftime.diff
    git stash save
    git pull --rebase
    if [ ! -d glogg ]; then
        mkdir -p glogg
        cp ~/.config/glogg/glogg_pattern.conf glogg/
    else
        count=$(sed -n 's/^searchPattern\\size=\([0-9]*\)/\1/p' glogg/glogg_pattern.conf)
        addtime=`date "+%Y-%m-%d_%H-%M-%S"`
        awk -v num=$count '{printf "%d:%s\n", NR+num , $0}' $difftime.diff | sed 's/\(^[0-9]*\):\([^\]*\\\)\([0-9]\+\)/\2\1/g' > $addtime.add
        addNum=$(echo $(cat $addtime.add|wc -l)+$count|bc)
        echo $addNum
        sed -i "s@\\(^searchPattern\\\size\\).*@\1=$addNum@" glogg/glogg_pattern.conf
        cat $addtime.add >> glogg/glogg_pattern.conf
    fi
    #git stash pop
    git add glogg/glogg_pattern.conf
    git add -u
    git commit -s -m "update patterns."
    branch=$(git rev-parse --abbrev-ref HEAD)
    git push origin -u HEAD:$branch
    cp glogg/glogg_pattern.conf ~/.config/glogg/glogg_pattern.conf
}

if [ -d ~/.glogg/$repoName ]; then
    uploadToRemote
else
    echo "Error: Directory does not exists."
    cd ~/.glogg/
    git clone $repoUrl
    if [ -d $repoName/glogg ]; then
        cp $repoName/glogg/glogg_pattern.conf ~/.config/glogg/glogg_pattern.conf
    else
        mkdir -p $repoName/glogg
        uploadToRemote
    fi
fi
