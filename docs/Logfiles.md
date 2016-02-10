![warning](./docs_warning.png)

When no **--ouput** option is given, logkeys logs to default log file `/var/log/logkeys.log`.

# Format #

Log files are UTF-8 encoded.

Each logging session  is  enclosed in "`Logging started...`" and "`Logging stopped at`
`<timestamp>`" strings. Whenever Enter key (Return key) or Ctrl+C or Ctrl+D combination is  pressed, a timestamp  is appended on a new line (provided **--no-timestamps** is not in effect).

Timestamp format is "%F %T%z", which results in "YYYY-mm-dd HH:MM:SS+ZZZZ". Time‐
stamp is separated from the logged keys by one '>' symbol.

All character key presses are logged as they appear. All function  key  presses are
replaced  with  strings as obtained from [keymap file](Keymaps.md), or as hardcoded when no keymap file is provided.

If a key is pressed down long enough so it repeats, it is logged only once and then
"`<#+DD>`" is appended, which hints the key was repeated DD more times. DD is a decimal figure, which is not to be taken absolutely correct.

If a keypress results in keycode, which is not recognized (i.e. key not found on a standard US or Intl 105-key keyboard), then the string "`<E-XX>`" is appended, where
XX is the received keycode in hexadecimal format. All  new  "WWW", "E-Mail", "Vol‐
ume+", "Media", "Help", etc. keys will result in this error string.

Using US keyboard layout, one example log file could look like:
```
Logging started ...

2009-12-11 09:58:17+0100 > lkl
2009-12-11 09:58:20+0100 > sudo cp <RShift>~/foo.<Tab> /usr/bin
2009-12-11 09:58:26+0100 > <LShift>R00<LShift>T_p455\\/0rD
2009-12-11 09:58:39+0100 > <Up><Up><Home>sudo
2009-12-11 09:58:44+0100 > c<#+53><BckSp><#+34><LCtrl>c
2009-12-11 09:58:54+0100 > lklk

Logging stopped at 2009-12-11 09:58:54+0100
```
If the same log was obtained by a logkeys process invoked with **--no-func-keys** option, it would look like:
```
Logging started ...

2009-12-11 09:58:17+0100 > lkl
2009-12-11 09:58:20+0100 > sudo cp ~/foo. /usr/bin
2009-12-11 09:58:26+0100 > R00T_p455\\/0rD
2009-12-11 09:58:39+0100 > sudo
2009-12-11 09:58:44+0100 > c<#+53>c
2009-12-11 09:58:54+0100 > lklk

Logging stopped at 2009-12-11 09:58:54+0100
```
Even when **--no-func-keys** is in effect, Space and Tab key presses are logged as a
single space character.
