#!/bin/bash


# Ensure only one instance is running at a time
create_lockfile() {
  OURPROCNAME="$(basename $0)"
  LOCKFILE="/tmp/.busy_$OURPROCNAME"
  LOCKAGE=30  #in minutes
  if (set -C; echo $$ > ${LOCKFILE} ) 2> /dev/null; then
    echo -n "Creating lockfile: ${LOCKFILE}      "
  else
    pid=$(cat ${LOCKFILE})
    # This is as specific of a test as we can get....REALLY!!!!
    if ps hp $pid > /dev/null; then
        #running
        #is it us and not something else?
        if ps hp $pid|grep -q "${OURPROCNAME}"; then
          #we're running, and it's us, lets test the age
          if [ -n "$(find ${LOCKFILE} -mmin +${LOCKAGE})" ]; then
            echo 
            # If we find that a process has been running for too long (lockfile exists for 30 minutes or more), kill the process and the lockfile
            echo "Found lockfile for process that has been running too long (30+ minutes)."
            echo "Killing process and removing lockfile: ${LOCKFILE}"
            echo
            kill -9 ${pid}
          else
            echo 
            echo "Cannot create Lockfile: $(basename $0) is already running [$(cat ${LOCKFILE})]"
            #generally this isn't an "error" case, so 0
            exit 0
          fi
        fi
    fi
    #the pid isn't ours, test for stuck.
    echo 
    echo "Found lockfile for process that is no longer running.  Removing lockfile: ${LOCKFILE}"
    echo
    rm -f ${LOCKFILE}
    echo -n "Creating lockfile: ${LOCKFILE}      "
    (set -C; echo $$ > ${LOCKFILE} ) 2> /dev/null
  fi
  # Automatically clean up the lock file
  # this would be better with add-to-trap logic...
  trap 'echo; echo "Removing lockfile: ${LOCKFILE}"; rm -f ${LOCKFILE}' EXIT SIGTERM
  echo_success 2>/dev/null|| echo "Success"
}
