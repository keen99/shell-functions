
Ever have a symlink that you want the real path for?  Or a worse, a relative symlink?

here's a classic example, thanks to brew:

    %% ls -l `which mvn`
    lrwxr-xr-x  1 draistrick  502  29 Dec 17 10:50 /usr/local/bin/mvn@ -> ../Cellar/maven/3.2.3/bin/mvn


use this function and it will return the -real- path:

	%% cat test.sh
	#!/bin/bash
	. resolve_path.inc
	echo
	echo "relative symlinked path:"
	which mvn
	echo
	echo "and the real path:"
	resolve_path `which mvn`


	%% test.sh

	relative symlinked path:
	/usr/local/bin/mvn

	and the real path:
	/usr/local/Cellar/maven/3.2.3/libexec/bin/mvn



BASH ONLY!

Tested on OSX (10.9, 10.6), FreeBSD (6.2-R), Centos (5.4, 6.5).  

Feedback, PRs, and travis integration for the tests welcome! :)
