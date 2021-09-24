#!/bin/bash
sed -i "1,$1d" $2
touch  $2
glogg $2
