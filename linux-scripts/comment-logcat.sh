#!/bin/bash
sed -i  "$1 s/\$/ \/\/$2/" $3
touch $3
glogg $3
