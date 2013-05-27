#!/bin/bash
# count code lines in current working directory
count=0
for file in *
do
	if [[ -d $file || $file = *.sh ]]; then
		continue
	fi
	n=$(wc -l < $file)
	let count=count+n
done
echo "current code lines is $count"
exit 0
	
