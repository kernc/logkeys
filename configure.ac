#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)
AC_INIT([logkeys],[0.1.1b-svn],[kerncece+logkeys@gmail.com],[],[http://code.google.com/p/logkeys/])
AC_CONFIG_SRCDIR([src/logkeys.cc])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_LANG([C++])

# More checks for programs.

AC_CHECK_PROG([FOUND_WHICH], [which], [yes], [no])
if test x"$FOUND_WHICH" = xno ; then
	AC_MSG_ERROR([Required program 'which' is missing.])
fi

AC_CHECK_PROG([FOUND_GREP], [grep], [yes], [no])
if test x"$FOUND_GREP" = xno ; then
	AC_MSG_ERROR([Required program 'grep' is missing.])
fi
AC_DEFINE_UNQUOTED([EXE_GREP], ["`which grep`"], [Holds path to grep executable])

AC_CHECK_PROG([FOUND_PS], [ps], [yes], [no])
if test x"$FOUND_PS" = xno ; then
	AC_MSG_ERROR([Required program 'ps' is missing.])
fi
AC_DEFINE_UNQUOTED([EXE_PS], ["`which ps`"], [Holds path to ps executable])

AC_CHECK_PROG([FOUND_DUMPKEYS], [dumpkeys], [yes], [no])
if test x"$FOUND_DUMPKEYS" = xno ; then
	AC_MSG_ERROR([Required program 'dumpkeys' is missing.])
fi
AC_DEFINE_UNQUOTED([EXE_DUMPKEYS], ["`which dumpkeys`"], [Holds path to dumpkeys executable])

# Checks for files

AC_CHECK_FILE(
  [/dev/input],
  [],
  [AC_MSG_ERROR([Input event interface devices not found in expected location /dev/input/eventX !])]
)
AC_CHECK_FILE(
  [/proc/bus/input/devices],
  [],
  [AC_MSG_ERROR([/proc/bus/input/devices is required to guess the correct keyboard input device.])]
)

# Checks for header files.

AC_CHECK_HEADERS(
  [cstdio cerrno cwchar cstring cassert sstream cstdlib csignal error.h unistd.h getopt.h sys/file.h sys/stat.h linux/input.h], 
  [],
  [AC_MSG_ERROR([Expected header file is missing!])]
)

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS(
  [geteuid error error_at_line exit on_exit memset setlocale strerror fprintf getopt_long fopen sscanf fscanf getpid getuid getgid fclose remove kill strlen strcat strcpy strncat freopen feof fgets atoi sigaction fork setsid open close flock write umask setegid seteuid strftime localtime fflush read time fgetws wcslen swscanf wcscpy popen pclose],
  [],
  [AC_MSG_ERROR([Expected function is missing!])]
)

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile scripts/Makefile])
AC_OUTPUT
