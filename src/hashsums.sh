#!/bin/bash

find . -name \* -type f | sort | \
while read f_name
do
  sha1sum -b "$f_name" >> ./sha1sums
done
./bin/axis -u"./"|grep -v axissums|grep -v sha1sums>./axissums
