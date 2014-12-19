#!/bin/bash

. create_lockfile.inc


#this should probably check the age and proc kill functions too..

run_test() {

	(
	echo "starting test"

	create_lockfile
	sleep 5
	echo "finished with lockfile"
	)
}


run_test &

run_test

echo "complete, waiting for background to finish."

wait