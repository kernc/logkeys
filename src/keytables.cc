/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: http://code.google.com/p/logkeys/
*/

#ifndef _KEYTABLES_H_
#define _KEYTABLES_H_

#include <cassert>
#include <linux/input.h>

namespace logkeys {

// these are ordered default US keymap keys
wchar_t char_keys[49] =  L"1234567890-=qwertyuiop[]asdfghjkl;'`\\zxcvbnm,./<";
wchar_t shift_keys[49] = L"!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"~|ZXCVBNM<>?>";
wchar_t altgr_keys[49] = {0}; // old, US don't use AltGr key: L"\0@\0$\0\0{[]}\\\0qwertyuiop\0~asdfghjkl\0\0\0\0zxcvbnm\0\0\0|";  // \0 on no symbol; as obtained by `loadkeys us`
// TODO: add altgr_shift_keys[] (http://en.wikipedia.org/wiki/AltGr_key#US_international)

wchar_t func_keys[][8] = {
  L"<Esc>", L"<BckSp>", L"<Tab>", L"<Enter>", L"<LCtrl>", L"<LShft>", L"<RShft>", L"<KP*>", L"<LAlt>", L" ", L"<CpsLk>", L"<F1>", L"<F2>", L"<F3>", L"<F4>", L"<F5>",
  L"<F6>", L"<F7>", L"<F8>", L"<F9>", L"<F10>", L"<NumLk>", L"<ScrLk>", L"<KP7>", L"<KP8>", L"<KP9>", L"<KP->", L"<KP4>", L"<KP5>", L"<KP6>", L"<KP+>", L"<KP1>",
  L"<KP2>", L"<KP3>", L"<KP0>", L"<KP.>", /*"<",*/ L"<F11>", L"<F12>", L"<KPEnt>", L"<RCtrl>", L"<KP/>", L"<PrtSc>", L"<AltGr>", L"<Break>" /*linefeed?*/, L"<Home>", L"<Up>", L"<PgUp>", 
  L"<Left>", L"<Right>", L"<End>", L"<Down>", L"<PgDn>", L"<Ins>", L"<Del>", L"<Pause>", L"<LMeta>", L"<RMeta>", L"<Menu>"
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

// translates character keycodes to continuous array indexes
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

// translates function keys keycodes to continuous array indexes
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

} // namespace logkeys

#endif  // _KEYTABLES_H_
