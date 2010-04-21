#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)
AC_INIT([logkeys],[0.1.0],[kerncece+logkeys@gmail.com],[],[http://code.google.com/p/logkeys/])
AC_CONFIG_SRCDIR([src/logkeys.cc])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_LANG([C++])

# Checks for programs.
AC_CHECK_PROG([FOUND_GREP], [grep], [yes], [no])
if test x"$FOUND_GREP" = xno ; then
	AC_MSG_ERROR([Required program grep is missing.])
fi
AC_CHECK_PROG([FOUND_PS], [ps], [yes], [no])
if test x"$FOUND_PS" = xno ; then
	AC_MSG_ERROR([Required program ps is missing.])
fi
AC_CHECK_PROG([FOUND_DUMPKEYS], [dumpkeys], [yes], [no])
if test x"$FOUND_DUMPKEYS" = xno ; then
	AC_MSG_ERROR([Required program dumpkeys is missing.])
fi

# Add --enable-evdev-path option
AC_ARG_ENABLE(
  [evdev-path],
  [  --enable-evdev-path=PATH  Search for input event devices in PATH], 
  [AC_DEFINE_UNQUOTED([INPUT_EVENT_PREFIX], ["$enableval"], [User defined input event device path])
    AC_CHECK_FILE(
      [$enableval],
      [],
      AC_MSG_ERROR([PATH provided in --enable-evdev-path does not exist. $enableval is not a valid directory!])
    )
  ]
)
# Add --enable-evdev option
AC_ARG_ENABLE(
  [evdev],
  [  --enable-evdev=DEV  Define input event device as DEV], 
  [AC_DEFINE_UNQUOTED([INPUT_EVENT_DEVICE], ["$enableval"], [User defined input event device])], 
  []
)
# Checks for files
AC_CHECK_FILE(
  [/dev/input],
  [],
  [AC_MSG_ERROR([Input event interface devices not at expected path! Use --enable-evdev-path to set the location of input event devices, e.g., ./configure --enable-evdev-path=/dev/input/ for event devices /dev/input/eventX.])]
)
AC_CHECK_FILE(
  [/proc/bus/input/devices],
  [],
  [AC_MSG_ERROR([/proc/bus/input/devices is required to guess the correct keyboard input device.])]
)

# Checks for header files.
AC_CHECK_HEADERS(
  [cstdio cerrno cstring sstream cstdlib csignal unistd.h getopt.h sys/file.h linux/input.h], 
  [],
  [AC_MSG_ERROR([Expected header missing!])]
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
  [geteuid getcwd memset setlocale strerror fprintf getopt_long fopen sscanf fscanf getpid fclose remove kill strlen strcat strcpy strncat freopen feof fgets atoi sigaction fork setsid open close flock write umask setgid setuid strftime localtime fflush read sleep time],
  [],
  [AC_MSG_ERROR([Expected function missing!])]
)

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile scripts/Makefile])
AC_OUTPUT
