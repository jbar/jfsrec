# jfsrec

jfsrec is a command line tool that performs a read-only extraction of files and directories from a jfs filesystem. It permits to recover files, if removed or if filesystem is damaged.

## A general recovery procedure
1. First of all, unmount the device or else more errors might be generated.
2. Create an image file, preferrably using dd_rhelp. dd_rhelp has many advantages over dd. Most notably, it minimizes the number of read operations and in the same time maximizes the amount of data that can be recovered. It also automatically restarts the copying process.
3. Use jfsrec on the image file. You need a mounted filesystem with at least as much free space as the files originally occupied. You will probably need some percents extra for spuriuos files.

## How jfsrec works
jfsrec works by first scanning for inodes, filtering them and then recover them, or more exactly: First an inode scan will be performed. The entire disk is scanned, and every 512-byte datablock is tested whether it might be a sane inode or not. If it is, it is saved for postprocessing at a later stage. This scan will take 2-3 hours on a 500Gb volume. Next, the inodes are postprocessed. The postprocessing filters out inodes that seems not to be sane. This stage is a multistep procedure. First the inodes are scanned for inode extents. An inode extent is a continous block of inodes. Inodes are always allocated in groups of 32, so all sane inodes should be a part of such an extent. Then the unallocated inodes, and duplicates, are removed. The case of multiple inostamps are not yet considered. When we have a set of presumably sane inodes the actual recovery starts. The recovery phase is also subdivided in several stages. First, the parents/child relationships are extracted for all directory inodes. Then the parents for all files are extracted, in the same time as all names for files and directories. Then the prioroties are calculated, and the roots are extracted. During these steps previously invalidated inodes may again be considered valid. This occurs if a file or directory shows a relationship to an inode that is not in the "sane" set, but in the invalidated set. The relationship extraction steps are surprisingly costly. Now the actual files and directories are recovered. First, the directory tree is created. Then each file is recreated. If an error occurs during the recovery of a file or directory the process skips to the next item. This kind of exhaustive search, as well as the recovery process, is quite costly operations, and will take a considerable time to complete. A recovery that takes a couple of hours is a fast one, days will be the normal case for larger volumes.

## How to use jfsrec
As jfsrec is still under development the exact usage will vary between versions. Please use 'jfsrec --help' for information.

    jfsrec usage: Allowed options:
      --help Displays a help message
      --output arg A directory where the extracted files are saved
      --logdir arg A directory where logfiles are saved
      --device arg The device from which files are to be extracted
      --blocksize arg (=4096) The blocksize of the device
      --first arg A grep compliant regex that specifies which files are to be recovered first
      --last arg A grep compliant regex that specifies which files are to be recovered last
      --exclude arg Exclude files that matches this perl compliant regex

A typical invocation would look like:

    % jfsrec --device /mnt/backup/the_trashed_image --output /mnt/terabyteraid/recovered --logdir /mnt/terabyteraid/logs --first jpg --last mpeg --exclude kde3

This will try to recover files from the image "/mnt/backup/the_trashed_image" and copy them to "/mnt/terabyteraid/recovered". Progress logs will be saved to "/mnt/terabyteraid/logs". These logs are used to continue the work if the process was interrupted (currently only in the scan and postprocess phases). Files matching the grep regex jpg will be recovered first, and files matching the regex mpeg will be recovered last. Files matching kde3 will not be recovered at all. Note that the regex may match anyware in the files full path. Path names may not be recovered for all files. If a it is not possible to recover the name of a file or directory its inode number will be used instead. 

## Huu... segfault on a given inode
Sometime segfault may occurs on some given inode (like this https://pastebin.com/raw/cGhZKZHm)
As jfsrec is not very active, and nobody has time to understand and fix this problem, there is a little bash script to help removed such inode from logfiles, so jfsrec will ignore them when re-invoqued with same options. Eg:

    % jfsrec_rm_logged_inode --logdir /mnt/terabyteraid/logs 8192

## How to compile
prerequisites:
* boost (www.boost.org/)
* a sane development environment (gcc+gmake+autoconf+automake)

Then cd into project dir, ./configure, make ...
