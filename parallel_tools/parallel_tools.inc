#!/bin/bash


set -e

bashv=${BASH_VERSION%.*}; bmajorv=${bashv%.*}; bminorv=${bashv#*.}
if [[ $bmajorv -lt 3 ]]
 then
  ##roughly off of the top of my head, this probably wont work with bash2...and maybe early versions of 3... definitely works with 3.2.57(1)-release
  echo "ERROR: "
  echo "ERROR: bash version 3.x required, currently running with ${bashv:-empty}"
  echo "ERROR: "
  exit 1
fi


printverbose() {
  #...
  echo "VERBOSE: $@"

}

printerror() {

  echo "ERROR: $@"

}

printnotice() {

  echo "NOTICE: $@"

}


 ##only use this one -last- after all the jobs are fired
parallelwait() {
  if [ x"${parallel:=}" != x ]
   then
    echo "waiting for parallel deploys...."
    wait
    wait
    echo "parallel complete"
  fi
}


### note - if your control-C these things, sane stuff does not happen.  someday maybe.  but I havent gotten the INT trap
### to DTRT.

parallel_run() {
#generally goes BEFORE testprint - and doesn't work if you eval into it.
#goes BEFORE wrapoutput, etc.  see ^eval.
  [[ -z ${PWAIT_TMP:=} ]] && export PWAIT_TMP=$(mktemp -d 2>/dev/null || mktemp -d -t 'ptmpdir')
  mkdir -p ${PWAIT_TMP}
  {
    ##BASHPID is a special now, set it and
    shit blows up
    MYBASHPID=$(bash -c 'echo $PPID')  #find this subshell's pid..
    ##echo PWAIT_TMP is $PWAIT_TMP and $MYBASHPID and $$
    trap_off 2>/dev/null || true
    echo "$@" > $PWAIT_TMP/${MYBASHPID}.cmd
    eval "$@"
    exit=$?
    echo $exit > $PWAIT_TMP/${MYBASHPID}.pid
    if [[ $exit -gt 0 ]]
     then
      printerror "parallel job failure on $MYBASHPID"
    fi
  } &
    #add the last background job to our pid list
    pwait_pids="${pwait_pids:=} $!"
}

#use this one to slow down job creation, limiting to $1 threads at a time - you'll want parallelwait to wait on the last <$1 jobs
parallel_wait_limiter() {
#..add some arg checking or..?
  if [[ "x${pwait_pids:=}" == "x" ]]
   then
    printerror "no pwait_pids in parallel_wait_limiter, did you call parallel_run first?"
    return 1
  fi

#    while [[ $(jobs -p | wc -l) -ge $1 ]]; do
    # only count running jobs, there are some edge cases where jobs -p (and jobs) continue returning
    # jobs that are "Done" long after they've finished.
    #44424
    #[1]+  Done                    { MYBASHPID=$(bash -c 'echo $PPID'); trap_off; echo "$@" > $PWAIT_TMP/${MYBASHPID}.cmd; eval "$@"; exit=$?; echo $exit > $PWAIT_TMP/${MYBASHPID}.pid; if [[ $exit -gt 0 ]]; then printerr "parallel job failure on $MYBASHPID";
    while [[ $(jobs |grep "Running"| wc -l) -ge $1 ]]; do
        sleep .5  #should work on osx and linux..
    done
}

#instead of parallelwait, use this last (only if you used _limit) because it handles failures..
parallel_wait_checkexit() {
  if [[ "x${pwait_pids:=}" == "x" ]]
   then
    printerror "no pwait_pids in parallel_wait_checkexit, did you call parallel_run first?"
    return 1
  fi

  if [[ ! "x$PWAIT_TMP" == "x" ]]
  then
    #wait will return the exit code from the pids
    #if any of them are nonzero, it'll return that..maybe
    printverbose "waiting on jobs to finish so we can error test them"
    #wait may error, so true it.
    wait || true ##$pwait_pids - they're long gone if they finished, so this gets noisy.
    printverbose "All jobs complete, testing for failures"
    fails=0
    if [[ $(ls -l $PWAIT_TMP/*.pid 2>/dev/null | wc -l)  -gt 0 ]]
     then
      for exitfile in $PWAIT_TMP/*.pid
       do
        code=$(cat $exitfile)
        if [[ $code -gt 0 ]]
         then
          let fails=fails+1
          exitcmdfile=$(echo ${exitfile}|sed 's/.pid//g')
          jobid=$(basename "$exitcmdfile")
          printerror "$jobid had an error: exit $code"
          printerror "$jobid job was [ $(cat ${exitcmdfile}.cmd) ]"
        fi
      done
      if [[ $fails -gt 0 ]]
       then
        rm -r $PWAIT_TMP || true
        printerror "We had parallel job failures on $fails of our jobs"
        return 1
      fi
    else
      [[ -d $PWAIT_TMP ]] && rm -r $PWAIT_TMP
      printerror "PWAIT_TMP directory is empty (or missing, or or or)"
    fi
  else
    rm -r $PWAIT_TMP || true
    printerror "ERROR: PWAIT_TMP var was empty!"
    return 1
  fi

  rm -r $PWAIT_TMP || true
  #now reset for the next time parallel_run gets called.
  unset PWAIT_TMP
  unset pwait_pids

}



parallel_wait_checkexit_condretry() {
  exitcodes=$@
  if [[ "x$exitcodes" == "x" ]]
   then
    printerror "FAIL: this function needs exit codes to check for!"
    return 1
  fi
  declare -a retryjobs
  if [[ "x${pwait_pids:=}" == "x" ]]
   then
    printerror "no pwait_pids in parallel_wait_checkexit_condretry, did you call parallel_run first?"
    return 1
  fi

  if [[ ! "x$PWAIT_TMP" == "x" ]]
  then
    #wait will return the exit code from the pids
    #if any of them are nonzero, it'll return that..maybe
    printverbose "waiting on jobs to finish so we can error test them"
    #wait may error, so true it.
    wait || true ##$pwait_pids - they're long gone if they finished, so this gets noisy.
    printverbose "All jobs complete, testing for failures"
    fails=0
    if [[ $(ls -l $PWAIT_TMP/*.pid 2>/dev/null | wc -l)  -gt 0 ]]
     then
      for exitfile in $PWAIT_TMP/*.pid
       do
        code=$(cat $exitfile)
        if [[ $code -gt 0 ]]
         then
          let fails=fails+1
          printerror "$exitfile has an error $code"
          exitcmdfile=$(echo ${exitfile}|sed 's/.pid//g')
          printerror "$exitfile job was $(cat ${exitcmdfile}.cmd)"
          for testcode in $exitcodes
           do
            if [[ $code == $testcode ]]
             then
              jobcmd="$(cat ${exitcmdfile}.cmd)"
              printnotice "Setting up $exitfile for retry"
              printnotice "Command was $jobcmd"
              retryjobs+=("$jobcmd")
            fi
          done
        fi
      done
      if [[ $fails -gt 0 ]]
       then
        rm -r $PWAIT_TMP || true
        printerror "We had parallel job failures on $fails of our jobs"
        ##return 1
      fi
    else
      [[ -d $PWAIT_TMP ]] && rm -r $PWAIT_TMP
      printerror "PWAIT_TMP directory is empty (or missing, or or or)"
    fi
  else
    rm -r $PWAIT_TMP || true
    printerror "ERROR: PWAIT_TMP var was empty!"
    return 1
  fi

  rm -r $PWAIT_TMP || true
  #now reset for the next time parallel_run gets called.
  unset PWAIT_TMP
  unset pwait_pids

  #now retry the retryables.
  jobsnum=${#retryjobs[*]}   ##correct syntax. the link is wrong!
  if [[ $jobsnum -gt 0 ]]
   then
    for ((i=0;i<$jobsnum;i++))
     do
      job="${retryjobs[${i}]}"
      printnotice "Retrying $job"
      parallel_run $job 2>&1
      parallel_wait_limiter $PARALLELTHREADS
    done
    printnotice "waiting for any leftover retry threads"
    ##..we could call back to ourselves and try forever if we want..
    parallel_wait_checkexit
    printnotice "done waiting for retrys, we did $loopcounter loops"
  fi
}




