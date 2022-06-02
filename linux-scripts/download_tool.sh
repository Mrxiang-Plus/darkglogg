url=$1
fileName=$2
cd ~/.glogg
curl $url --output $fileName
