AC_INIT([jfsrec], [0.2])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_LANG_CPLUSPLUS
AC_PROG_CXX
dnl AM_PROG_LIBTOOL

AM_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_CHECK_HEADERS(boost/progress.hpp)

if test "$ac_cv_header_boost_progress_hpp" == no
then
	AC_MSG_ERROR([Error, boost/progress.hpp header is missing!])
fi

AC_CHECK_HEADERS(boost/regex.hpp)

if test "$ac_cv_header_boost_regex_hpp" == no
then
	AC_MSG_ERROR([Error, boost/regex.hpp header is missing!])
fi

AC_CHECK_HEADERS(boost/filesystem/path.hpp)

if test "$ac_cv_header_boost_filesystem_path_hpp" == no
then
	AC_MSG_ERROR([Error, boost/filesystem/path.hpp header is missing!])
fi

AC_CHECK_HEADERS(boost/program_options.hpp)

if test "$ac_cv_header_boost_program_options_hpp" == no
then
	AC_MSG_ERROR([Error, boost/program_options.hpp header is missing!])
fi

AC_OUTPUT
