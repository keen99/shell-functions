#!/bin/bash


#works now.







PARALLELTHREADS=4  #this should use some serverdb magic..

. ../trap_on/trap.inc

trap_on

    parallel="&"  ##without this, parallelwait doesn't wait.

. parallel_tools.inc
. wrapoutput.inc

echo_and_sleep() {

	echo "$@ start and sleep"
	sleep 5
	echo "$@ done sleep"

}



echo "wrapoutput test"
wrapoutput "FISH:" echo test

#exit
for server in one two three four five six seven eight nine 10
 do


            shortname=$(echo $server | awk -F. '{print $1}')

            parallel_run wrapoutput $shortname echo_and_sleep $server
            parallel_wait_limiter $PARALLELTHREADS
done

            printnotice "waiting for any leftover threads"
            parallel_wait_checkexit

