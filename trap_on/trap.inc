#!/bin/sh


set_trap() {
	trap 'my_trap_handler ${LINENO} $? "${BASH_COMMAND}"' ERR
	set +e  #make sure -e is off.
	set -u #unbound..
	# Let shell functions inherit ERR trap.  Same as `set -E'.
	set -o errtrace
}
function my_trap_handler() {
        MYSELF="$(basename $0)"               # equals to my script name
        LASTLINE="$1"            # argument 1: last line of error occurence
        LASTERR="$2"             # argument 2: error code of last command
        LASTCMD="$3"            # arg 3: the actual last command
        echo "ERROR ${MYSELF}:${LASTLINE}: command: [ $LASTCMD ]"
#        eval echo "ERROR ${MYSELF}:${LASTLINE}: command expanded: [ $LASTCMD ]"

        SAFELASTCMD=$( echo "$LASTCMD"|sed 's/\$\((.*)\)/subshell[\1]/' )
        eval echo \""ERROR ${MYSELF}:${LASTLINE}: command expanded: [ $SAFELASTCMD ]\""
        echo "ERROR ${MYSELF}:${LASTLINE}: exit code: ${LASTERR}"
        #either exit here, or let set -e do it - there are potential subshell issues either way..
        #for example, set -e will block an exit trap from running...well, mayne not. though I'd seen it cant repo
        exit $LASTERR
}


cond_trap_on() {
	#if it's be called once, then turn it back on. otherwise dont
	if [ ${TRAPON:=0} = 1 ]
	 then
		trap_on
	fi
}

trap_on() {
	#save this for later.
	[[ -z ${PARENTPID:=} ]] && export PARENTPID=$$
	export TRAPON=1
echo "TRAP ON!"
	set_trap
}

trap_off() {
	#make it conditional so we dont unset traps not created by trap_on
	if [ ${TRAPON:=0} = 1 ]
	 then
	  # This has been fixed with the addition of set -o errtrace in trap_on
	  set +e  #dont unset E here, we want to keep that one around.
	  #set +u #unsafe? not here?
	  trap - 1 2 3 15 ERR
	  return 0
	fi
}

