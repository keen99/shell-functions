#!/bin/bash

set -u
set -e

. ./resolve_path.inc

testit() {
	if [ "$1" = "$2" ]
	 then
		echo OK
	else
		echo "FAILED! no match"
		echo "[$1]"
		echo "[$2]"
		exit 1
	fi	
	SUCCESSCOUNT=${SUCCESSCOUNT:=0}
	let SUCCESSCOUNT=SUCCESSCOUNT+1
}
echo test cases.

OWD=$PWD

temp=`mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir'`
mkdir -p "$temp"
cd "$temp"
#because on OSX, these are in a symlinked tree...
temp=$(pwd -P)
echo "temp=$temp"


#now go do some tests.


echo "TEST: no such file/dir"
	cd $temp
	result=$(resolve_path missingfile 2>&1||true)
	echo "$result"
	#have to filter our result a bit due to possible debug on stderr
	testit "$(echo "$result" | grep "No such")" "resolve_path: missingfile: No such file or directory"

echo "TEST: just a file"
	cd $temp
	touch file
	result=$(resolve_path file)
	echo "$result"
	rm -f file
	testit "$result" "$temp/file"

echo "TEST: just a dir"
	cd $temp
	mkdir dir
	result=$(resolve_path dir)
	echo "$result"
	rm -rf dir
	testit "$result" "$temp/dir"

echo "TEST: symlink to file"
	touch file
	#ls -la
	ln -s file link 
	ls -la link
	short=$(resolve_path link)
	echo "$short"
	cd $OWD
	long=$(resolve_path $temp/link)
	cd $temp
	echo "$long"
	rm -f file link
	testit "$short" "$long"

echo "TEST: symlink to symlink file:"
	cd $temp
	touch file
	ln -s file link
	ln -s link link2
	ls -la link link2
	result=$(resolve_path link2)
	echo "$result"
	rm -f file link link2
	testit "$result" "$temp/file"

echo "TEST: symlink to dir"
	cd $temp
	mkdir dir
	ln -s dir link
	ls -la link
	result=$(resolve_path link)
	echo "$result"
	rm -rf dir link
	testit "$result" "$temp/dir"

echo "TEST: symlink to symlink dir:"
	cd $temp
	mkdir dir
	ln -s dir link
	ln -s link link2
	ls -la link link2
	result=$(resolve_path link2)
	echo "$result"
	rm -rf dir link link2
	testit "$result" "$temp/dir"

echo "TEST: symlink to file, relative"
	cd $temp
	mkdir -p dir/
	touch dir/file
	cd dir/
	ln -s file link
	cd $temp
	ls -la dir/file dir/link
	result=$(resolve_path dir/link)
	echo "$result"
	rm -rf dir
 	testit "$result" "$temp/dir/file"

echo "TEST: symlink to file in dir, relative with dots"
	cd $temp
	mkdir -p realdir/
	mkdir -p linkdir/
	touch realdir/file
	cd linkdir/
	ln -s ../realdir/file link
	cd $temp
	ls -la realdir/file linkdir/link
	result=$(resolve_path linkdir/link)
	echo "$result"
	rm -rf realdir linkdir
 	testit "$result" "$temp/realdir/file"	

echo "TEST: symlink to dir, relative"
	cd $temp
	mkdir -p dir/
	mkdir -p dir/dir2
	cd dir/
	ln -s dir2 link
	cd $temp
	ls -la dir/dir2 dir/link
	result=$(resolve_path dir/link)
	echo "$result"
	rm -rf dir
 	testit "$result" "$temp/dir/dir2"

echo "TEST: symlink to dir, relative with dots"
	cd $temp
	mkdir -p realdir/
	mkdir -p linkdir/
	mkdir -p realdir/dir
	cd linkdir/
	ln -s ../realdir/dir link
	cd $temp
	ls -la realdir/dir linkdir/link
	result=$(resolve_path linkdir/link)
	echo "$result"
	rm -rf realdir linkdir
 	testit "$result" "$temp/realdir/dir"	

echo
echo "Successful tests: $SUCCESSCOUNT"
exit 0

