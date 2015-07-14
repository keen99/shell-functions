#!/bin/sh

##note: not so good at wrapping function calls though. they bork at the eval..
wrapoutput() {
# usage - wrapthings withwhat runwhat
# wraps the output of the command given with the tag that's given
# will exec in a subshell, so be warned about variable scoping coming back out.
# great for sshes though
#there could be some quote goofys going on still
	wrap=$1
	shift
	##echo wrapping $wrap around "$@"
#should probably reset to what this was before..
	set -o pipefail #we want the eval, not the while read...
	eval $@ | while read i
	 do
	  echo "$wrap: $i";
	done
	local exit=$?
	set +o pipefail
	return $exit
}  

  