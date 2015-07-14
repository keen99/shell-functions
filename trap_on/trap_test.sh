#!/bin/bash

. trap.inc

trap_on

success() {
  echo "test with success"
}

##so this case needs some work.  return's dont show us really what we want..I guess they do, but..
failure() {
  echo "test with failure"
  varexpansion="var expansion test"
  echo "test var expansion: $varexpansion"
  return 198
}



echo "this should be happy"
success

false expanded $BASHPID and subshell $(echo this is BAD)

tmpdir=$(mktemp -d 2>/dev/null || mktemp -d -t 'ptmpdir')
mkdir -p $tmpdir
echo '#!/bin/bash
  echo "test with failure"
  varexpansion="var expansion test"
  echo "test var expansion: $varexpansion"
  exit 199' > $tmpdir/testrun
chmod +x $tmpdir/testrun
$tmpdir/testrun

echo "this should be sadder"



echo "this should be sad"
failure


