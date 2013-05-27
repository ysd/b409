#!/bin/bash
fa=a.c
fb=b.c
fc=c.c
fd=f.c
set -- $(echo "$fa $fb $fc $fd")
echo "$# args"
for file
do
	echo "creat file $file"
	./_creat "$file"
done
for file
do
	echo "write file $file"
	./_write "$file" ssssssssssssssssssss
done
exit 0
