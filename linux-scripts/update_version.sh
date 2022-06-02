url=$1
fileName=`basename $1`
cd ~/.glogg
echo "file>>>" $fileName
echo "url>>>>" $url
wget -O $fileName $url
#wget -O glogg_ubuntu_V1.0.tar.gz https://git.n.xiaomi.com/wanghuiting1/gloggversion/-/raw/master/Download/Ubuntu/glogg_ubuntu_V1.0.tar.gz

tar -xvf $fileName
cd release
./install.sh
rm $fileName



