#!/bin/bash


##pulled out of golf...
## includes maven...

unalias which 2> /dev/null || true
function which () { builtin type -pa "$@" | head -1; }



function setjdk() {
  set -u
    if [ $# -ne 0 ]; then
      #remove the osx home..
      #should probably do some logics.
        removeFromPath '/System/Library/Frameworks/JavaVM.framework/Home/bin'
      #when switching versions, we need to dump the old one from the path
      if [ -n "${JAVA_HOME:=}" ]; then
          removeFromPath $JAVA_HOME/bin
      fi
    # use the java_home tool if we can. (OSX)
    #check our path.
    if builtin type -pa "java_home" > /dev/null
     then
     java_home_bin=$(builtin type -pa "java_home" | head -1)
    elif [ -e /usr/libexec/java_home ]
     then
      java_home_bin="/usr/libexec/java_home"
    #we cant use locate everywhere, some linuxes dont let users run it.
    # elif locate libexec/java_home|grep "libexec/java_home"
    #  then
    #   java_home_bin=locate libexec/java_home|grep "libexec/java_home"|head -n1
    elif [ -e /etc/alternatives/java ]
     then
      #can we use the alterntiave link to find versions..?
      JAVA_DIR="$(dirname $(dirname $(readlink /etc/alternatives/java)))"
      #=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.25-3.b17.el6_6.x86_64/jre/
      java_root=$(dirname $(dirname $JAVA_DIR))
      #=/usr/lib/jvm/
      java_bin=$(ls -1 $java_root/*$@*/bin/java|head -n1)
      #=/usr/lib/jvm/java-1.8.0/bin/java
    elif builtin type -pa "java" > /dev/null
     then
        java_bin=$(builtin type -pa "java" | head -1)
    else
      echo "ERROR: unable to set JAVA_HOME, we can't find it for $@"
      return 1
    fi
    if [ -n "${java_home_bin:=}" ]
     then
      JAVA_HOME=$($java_home_bin -v $@)
    elif [ -n "$java_bin" ]
     then
      JAVA_HOME=$(dirname $(dirname $java_bin))
    else
      echo "ERROR: unable to set JAVA_HOME, we cant find it for $@ (2)."
      return 1
    fi
    #now lets get the absolute path of whatever we found.
    export JAVA_HOME=$(resolve_path "$JAVA_HOME")
        export PATH=$JAVA_HOME/bin:$PATH
    else
      echo "$FUNCNAME requires a version argument - ie: 1.5, 1.6, 1.7, 1.8"
      return 1
    fi
}
function removeFromPath() {
    export PATH=$(echo $PATH | sed -E -e "s;:$1;;" -e "s;$1:?;;")
}

setmvn() {
  set -u
  #unset this if it's set to avoid issues
  export MAVEN_BIN=""
  export MAVEN_HOME=""
    if [ -n "${MAVEN_HOME:=}" ]; then
echo "removing old MAVEN_HOME = $MAVEN_HOME"
        removeFromPath $MAVEN_HOME/bin
    fi
  #requires resolve_path
  #
  #find a mvn binary
  if [ -e /etc/alternatives/mvn ]
     then
      MAVEN_BIN="/etc/alternatives/mvn"
  elif builtin type -pa "mvn" > /dev/null
   then
    MAVEN_BIN=$(builtin type -pa "mvn" | head -1)
  elif [ -e "/usr/local/maven/bin/mvn" ]
   then
    MAVEN_BIN="/usr/local/maven/bin/mvn"
  else
    # no locate, not always accessable w/o root on linux. :(
    # #last ditch attempt to find something!
    # MAVEN_BIN=$(locate bin/mvn|grep "bin/mvn$"|grep -v "libexec"|head -n1)
    if [ -n $MAVEN_BIN ]
     then
      echo "ERROR: unable to locate maven (mvn) binary, stopping."
      return 1
    fi
  fi
  #chase any symlinks and relative paths
  MAVEN_BIN=$(resolve_path "$MAVEN_BIN")
  export MAVEN_HOME=$(dirname $(dirname "$MAVEN_BIN"))
  export PATH="$MAVEN_HOME/bin:$PATH"
}


resolve_path() {
  set -u
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
  if [[ -L "$opath" ]]
   then
  #it's a link.
  #file or directory, we want to cd into it's dir
    cd $odir
  #then extract where the link points.
    npath=$(readlink "$obase")
    #have to -L BEFORE we -f, because -f includes -L :(
    if [[ -L $npath ]]
     then
    #the link points to another symlink, so go follow that.
      resolve_path "$npath"
      #and finish out early, we're done.
      return $?
      #done
    elif [[ -f $npath ]]
    #the link points to a file.
     then
      #get the dir for the new file
      nbase=$(basename $npath)
      npath=$(dirname $npath)
      cd "$npath"
      ndir=$(pwd -P)
      retval=0
      #done
    elif [[ -d $npath ]]
     then
    #the link points to a directory.
      cd "$npath"
      ndir=$(pwd -P)
      retval=0
      #done
    else
      echo "$FUNCNAME: ERROR: unknown condition inside link!!" >&2
      echo "opath [[ $opath ]]" >&2
      echo "npath [[ $npath ]]" >&2
      return 1
    fi
  else
    if ! [[ -e "$opath" ]]
     then
      echo "$FUNCNAME: $opath: No such file or directory" >&2
      return 1
      #and break early
    elif [[ -d "$opath" ]]
     then
      cd "$opath"
      ndir=$(pwd -P)
      retval=0
      #done
    elif [[ -f "$opath" ]]
     then
      cd $odir
      ndir=$(pwd -P)
      nbase=$(basename "$opath")
      retval=0
      #done
    else
      echo "$FUNCNAME: ERROR: unknown condition outside link!!" >&2
      echo "opath [[ $opath ]]" >&2
      return 1
    fi
  fi
  #now assemble our output
  echo -n "$ndir"
  if [[ "x${nbase:=}" != "x" ]]
   then
    echo "/$nbase"
  else
    echo
  fi
  #now return to where we were
  cd "$owd"
  return $retval
}

