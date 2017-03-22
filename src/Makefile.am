bin_PROGRAMS = jfsrec
jfsrec_SOURCES = jfsrec.cpp options.cpp inodevector.cpp inode.cpp dirinode.cpp \
	fileinode.cpp inodefactory.cpp device.cpp inodescanner.cpp recoverer.cpp \
	unicode_to_utf8.cpp unallocatedinode.cpp symlinkinode.cpp specialinode.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
jfsrec_LDFLAGS = $(all_libraries) 
noinst_HEADERS = inode.h inodescanner.h options.h inodevector.h fileinode.h \
	dirinode.h device.h inodefactory.h recoverer.h unicode_to_utf8.h \
	unallocatedinode.h symlinkinode.h specialinode.h
jfsrec_LDADD = -lboost_regex -lboost_program_options -lboost_filesystem
