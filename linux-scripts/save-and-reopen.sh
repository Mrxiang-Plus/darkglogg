#! /bin/bash
cp $1 $2
glogg -m $2
open -a "glogg" --args -m $2
