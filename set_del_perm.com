$! set owner delete permission on all files in epics tree
$ myproc=f$environment("PROCEDURE")
$ myroot=f$parse("A.A;1",myproc,,,"SYNTAX_ONLY") - "]A.A;1" + ".]"
$ set file/prot=(o:rwd) 'myroot'[000000...]*.*;* 
$! set file/prot=(o:rwd) 'myroot'[000000...]*.*;* /exclude=(*.dir;1)
