#!/bin/bash
#repoUrl="git@git.n.xiaomi.com:MiuiCamera/miuicameratool.git"
#repoUrl="/pc2/work/test/.git/"

echo "repo url:>>>>"$1
repoUrl=$1
repoName=`basename $repoUrl .git`
#repoName="test"

function uploadToRemote() {
    cd ~/.glogg/$repoName
    if [ ! -d glogg ]; then
        mkdir -p glogg
    fi
    cp ~/.config/glogg/glogg_pattern.ini .

    grep "searchPattern\\\[0-9]\+\\\string=" glogg_pattern.ini > pattern.txt
    sed -i "s/\([^\]\)\"/\1/g" pattern.txt
    sed -i "s/searchPattern\\\[0-9]\+\\\string=[\"]*//g" pattern.txt
    sed -i '/^$/d' pattern.txt
    sort pattern.txt | sed '$!N; /^\(.*\)\n\1$/!P; D' > glogg_pattern.txt
    rm glogg_pattern.ini


    if [ ! -d glogg ]; then
        mkdir -p glogg
        cp glogg_pattern.txt glogg/
    else
        cp glogg_pattern.txt glogg/
        difftime=`date "+%Y-%m-%d_%H-%M-%S"`
        git diff|grep "^+[^+]"  > $difftime.diff
        sed -i "s/^+//g" $difftime.diff
        sed -i "s/\([^\]\)\"/\1/g" $difftime.diff
        git stash save
        git pull --rebase
        if [ -s glogg/glogg_pattern.txt ]
        then
            rm pattern.txt
            cat $difftime.diff >> glogg/glogg_pattern.txt
        else
            mv pattern.txt  glogg/glogg_pattern.txt
        fi

        if [ ! -s $difftime.diff ]
        then
           rm $difftime.diff
        fi
        sort glogg/glogg_pattern.txt| sed '$!N; /^\(.*\)\n\1$/!P; D' > tmp.txt
        mv tmp.txt glogg/glogg_pattern.txt

        awk '{print "searchPattern\\" NR "\\string=\"" $s "\""}' glogg/glogg_pattern.txt > glogg_pattern.txt
        echo "[SavedPatterns]" > tmp.txt
        cat glogg_pattern.txt >> tmp.txt
        count=$(cat glogg_pattern.txt|wc -l)
        echo "searchPattern\\size=$count" >> tmp.txt
        echo "version=1" >> tmp.txt
        mv tmp.txt ~/.config/glogg/glogg_pattern.ini
        rm glogg_pattern.txt
    fi
    #git stash pop
    git add glogg/glogg_pattern.txt
    git add -u
    git commit -s -m "update patterns."
    branch=$(git rev-parse --abbrev-ref HEAD)
    git push origin -u HEAD:$branch
}

if [  ! -d ~/.glogg/$repoName ]; then
    echo "Error: Directory does not exists."
    cd ~/.glogg/
    git clone $repoUrl
    if [ ! -d $repoName/glogg ]; then
        mkdir -p $repoName/glogg
    fi
fi
uploadToRemote
