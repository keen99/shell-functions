#!/bin/bash

resolve_path() {
	#I'm bash only, please!
	# usage:  resolve_path <a file or directory> 
	# follows symlinks and relative paths, returns a full real path
	#
	local owd="$PWD"
	#echo "$FUNCNAME for $1" >&2
	local opath="$1"
	local npath=""
	local obase=$(basename "$opath")
	local odir=$(dirname "$opath")
	if [ -L "$opath" ]
	then
	#it's a link.
	#file or directory, we want to cd into it's dir
		cd $odir
	#then extract where the link points.
		npath=$(readlink "$obase")
		#have to -L BEFORE we -f, because -f includes -L :(
		if [ -L $npath ]
		 then
		#the link points to another symlink, so go follow that.
			resolve_path "$npath"
			#and finish out early, we're done.
			return $?
			#done
		elif [ -f $npath ]
		#the link points to a file.
		 then
			#get the dir for the new file
			nbase=$(basename $npath)
		 	npath=$(dirname $npath)
		 	cd "$npath"
		 	ndir=$(pwd -P)
		 	retval=0
		 	#done
		elif [ -d $npath ]
		 then
		#the link points to a directory.
			cd "$npath"
			ndir=$(pwd -P)
			retval=0
			#done
		else
			echo "$FUNCNAME: ERROR: unknown condition inside link!!" >&2
			echo "opath [ $opath ]" >&2
			echo "npath [ $npath ]" >&2
			return 1
		fi
	else
		if ! [ -e "$opath" ]
		 then
			echo "$FUNCNAME: $opath: No such file or directory" >&2
			return 1
			#and break early
		elif [ -d "$opath" ]
		 then 
			cd "$opath"
			ndir=$(pwd -P)
			retval=0
			#done
		elif [ -f "$opath" ]
		 then
		 	cd $odir
		 	ndir=$(pwd -P)
		 	nbase=$(basename "$opath")
		 	retval=0
		 	#done
		else
			echo "$FUNCNAME: ERROR: unknown condition outside link!!" >&2
			echo "opath [ $opath ]" >&2
			return 1
		fi
	fi
	#now assemble our output
	echo -n "$ndir"
	if [ "x${nbase:=}" != "x" ]
	 then
	 	echo "/$nbase"
	else 
		echo
	fi
	#now return to where we were
	cd "$owd"
	return $retval
}