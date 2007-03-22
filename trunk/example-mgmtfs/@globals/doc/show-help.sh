#!/bin/bash
shift
path="../.."
for part in $@
do
	path="$path/$part"
done

if [ ! -d $path ]
then
	echo "'$@' command not found."
	exit 2
fi

doc_file=$path/@doc
if [ -f $doc_file ]
then
	less $doc_file
else
	echo "no documentation for '$@'."
	exit 1
fi

