
```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vpk -?
LoadLibrary: pModule: filesystem_stdio, path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
Usage: vpk [options] <command> <command arguments ...>
       vpk [options] <directory>
       vpk [options] <vpkfile>

CREATE VPK / ADD FILES:
  vpk <dirname>
         Creates a pack file named <dirname>.vpk located
         in the parent of the specified directory.
  vpk a <vpkfile> <filename1> <filename2> ...
         Add file(s).
  vpk a <vpkfile> @<filename>
         Add files listed in a response file.
  vpk k <vpkfile> <keyvalues_filename>
         Add files listed in a keyvalues control file.
  vpk <directory>
         Create VPK from directory structure.  (This is invoked when
         a directory is dragged onto the VPK tool.)

EXTRACT FILES:
  vpk x <vpkfile> <filename1> <filename2> ...
         Extract file(s).
  vpk <vpkfile>
         Extract all files from VPK.  (This is invoked when
         a .VPK file is dragged onto the VPK tool.)

DISPLAY VPK INFO:
  vpk l <vpkfile>
         List contents of VPK.
  vpk L <vpkfile>
         List contents (detailed) of VPK.


Options:
  -v     Verbose.
  -M     Produce a multi-chunk pack file
  -P     Use SteamPipe-friendly incremental build algorithm.
         Use with 'k' command.
         For optimal incremental build performance, the control file used
         for the previous build should exist and be named the same as the
         input control file, with '.bak' appended, and each file entry
         should have an 'md5' value.  The 'md5' field need not be the
         actual MD5 of the file contents, it is just a unique identifier
         that will be compared to determine if the file contents has changed
         between builds.
         This option implies -M
  -c <size>
         Use specified chunk size (in MB).  Default is 200.
  -a <align>
         Align files within chunk on n-byte boundary.  Default is 1.
```

#### openssl

openssl version does not work, there're a few undefined variables not found anywhere

