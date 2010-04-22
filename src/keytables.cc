#ifndef _DEFAULT_KEYS_H_
#define _DEFAULT_KEYS_H_

#include <cassert>
#include <linux/input.h>

// these are ordered default US keymap keys
wchar_t char_keys[49] =  L"1234567890-=qwertyuiop[]asdfghjkl;'`\\zxcvbnm,./<";
wchar_t shift_keys[49] = L"!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"~|ZXCVBNM<>?>";
wchar_t altgr_keys[49] = {0}; // old, US don't use AltGr key: L"\0@\0$\0\0{[]}\\\0qwertyuiop\0~asdfghjkl\0\0\0\0zxcvbnm\0\0\0|";  // \0 on no symbol; as obtained by `loadkeys us`
// TODO: add altgr_shift_keys[]

char func_keys[][8] = {
  "<Esc>", "<BckSp>", "<Tab>", "<Enter>", "<LCtrl>", "<LShft>", "<RShft>", "<KP*>", "<LAlt>", " ", "<CpsLk>", "<F1>", "<F2>", "<F3>", "<F4>", "<F5>",
  "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<NumLk>", "<ScrLk>", "<KP7>", "<KP8>", "<KP9>", "<KP->", "<KP4>", "<KP5>", "<KP6>", "<KP+>", "<KP1>",
  "<KP2>", "<KP3>", "<KP0>", "<KP.>", /*"<",*/ "<F11>", "<F12>", "<KPEnt>", "<RCtrl>", "<KP/>", "<PrtSc>", "<AltGr>", "<Break>" /*linefeed?*/, "<Home>", "<Up>", "<PgUp>", 
  "<Left>", "<Right>", "<End>", "<Down>", "<PgDn>", "<Ins>", "<Del>", "<Pause>", "<LMeta>", "<RMeta>", "<Menu>"
};

const char char_or_func[] =  // c = character key, f = function key, _ = blank/error ('_' is used, don't change); all according to KEY_* defines from <linux/input.h>
  "_fccccccccccccff"
  "ccccccccccccffcc"
  "ccccccccccfccccc"
  "ccccccffffffffff"
  "ffffffffffffffff"
  "ffff__cff_______"
  "ffffffffffffffff"
  "_______f_____fff";
#define N_KEYS_DEFINED 106  // sum of all 'c' and 'f' chars in char_or_func[]

inline bool is_char_key(unsigned int code)
{
  assert(code < sizeof(char_or_func));
  return (char_or_func[code] == 'c');
}

inline bool is_func_key(unsigned int code)
{
  assert(code < sizeof(char_or_func));
  return (char_or_func[code] == 'f');
}

inline bool is_used_key(unsigned int code)
{
  assert(code < sizeof(char_or_func));
  return (char_or_func[code] != '_');
}

// translates character keycodes to continuous array indices
inline int to_char_keys_index(unsigned int keycode)
{
  if (keycode >= KEY_1 && keycode <= KEY_EQUAL)  // keycodes 2-13: US keyboard: 1, 2, ..., 0, -, =
    return keycode - 2;
  if (keycode >= KEY_Q && keycode <= KEY_RIGHTBRACE)  // keycodes 16-27: q, w, ..., [, ]
    return keycode - 4;
  if (keycode >= KEY_A && keycode <= KEY_GRAVE)  // keycodes 30-41: a, s, ..., ', `
    return keycode - 6;
  if (keycode >= KEY_BACKSLASH && keycode <= KEY_SLASH)  // keycodes 43-53: \, z, ..., ., /
    return keycode - 7;
  
  if (keycode == KEY_102ND) return 47;  // key right to the left of 'Z' on US layout
  
  return -1;  // not character keycode
}

// translates function keys keycodes to continuous array indices
inline int to_func_keys_index(unsigned int keycode)
{
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

#endif
