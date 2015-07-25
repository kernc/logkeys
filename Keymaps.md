
<br />


&lt;hr/&gt;



If neither **--us-keymap** switch nor **--keymap** switch are used, then logkeys determines the keymap automatically with the help of **dumpkeys** command. For it to work even slightly reliably (mapping correct character), [console kernel keymap needs to be set](Documentation#Installation.md).

If you are using US layout keyboard and need default US keymap, run logkeys with **--us-keymap** switch.

# Format #

The keymap file is expected to be UTF-8 encoded.

Each line of file represents either one character key or one function key.  The for‐
mat specifies at least two and up to three space-delimited characters  on  character
key lines (first character without modifiers, second with Shift in action, optional third with
AltGr in action), and up to 7 characters long string on function key lines.

The [keymap for Slovene keyboard layout](Keymaps#Download.md) (also Croatian) would look like:
(note, line numbers are printed for convenience only)
<table width='750px'>
<blockquote><tr>
<blockquote><td width='160' valign='top'>
<wiki:gadget up_ad_client="2900001379782823" up_ad_slot="4184917647" width="160" height="600" border="0" up_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" /><br>
</td>
<td valign='top'>
<pre><code>  1: &lt;Esc&gt;<br>
  2: 1 ! ~<br>
  3: 2 " ˇ<br>
  4: 3 # ^<br>
  5: 4 $ ˘<br>
  6: 5 % °<br>
  7: 6 &amp; ˛<br>
  8: 7 / `<br>
  9: 8 ( ˙<br>
 10: 9 ) ´<br>
 11: 0 = ˝<br>
 12: ' ? ¨<br>
 13: + * ¸<br>
 14: &lt;BckSp&gt;<br>
 15: &lt;Tab&gt;<br>
 16: q Q \<br>
 17: w W |<br>
 18: e E €<br>
 19: r R<br>
 20: t T<br>
 21: z Z<br>
 22: u U<br>
 23: i I<br>
 24: o O<br>
 25: p P<br>
 26: š Š ÷<br>
 27: đ Đ ×<br>
 28: &lt;Enter&gt;<br>
 29: &lt;LCtrl&gt;<br>
 30: a A<br>
 31: s S<br>
 32: d D<br>
 33: f F [<br>
 34: g G ]<br>
 35: h H<br>
 36: j J<br>
 37: k K ł<br>
 38: l L Ł<br>
 39: č Č<br>
 40: ć Ć ß<br>
 41: ¸ ¨<br>
 42: &lt;LShft&gt;<br>
 43: ž Ž ¤<br>
 44: y Y<br>
 45: x X<br>
 46: c C<br>
 47: v V @<br>
 48: b B {<br>
 49: n N }<br>
 50: m M §<br>
 51: , ; &lt;<br>
 52: . : &gt;<br>
 53: - _<br>
 54: &lt;RShft&gt;<br>
 55: &lt;KP*&gt;<br>
 56: &lt;LAlt&gt;<br>
 57:<br>
 58: &lt;CpsLk&gt;<br>
 59: &lt;F1&gt;<br>
 60: &lt;F2&gt;<br>
 61: &lt;F3&gt;<br>
 62: &lt;F4&gt;<br>
 63: &lt;F5&gt;<br>
 64: &lt;F6&gt;<br>
 65: &lt;F7&gt;<br>
 66: &lt;F8&gt;<br>
 67: &lt;F9&gt;<br>
 68: &lt;F10&gt;<br>
 69: &lt;NumLk&gt;<br>
 70: &lt;ScrLk&gt;<br>
 71: &lt;KP7&gt;<br>
 72: &lt;KP8&gt;<br>
 73: &lt;KP9&gt;<br>
 74: &lt;KP-&gt;<br>
 75: &lt;KP4&gt;<br>
 76: &lt;KP5&gt;<br>
 77: &lt;KP6&gt;<br>
 78: &lt;KP+&gt;<br>
 79: &lt;KP1&gt;<br>
 80: &lt;KP2&gt;<br>
 81: &lt;KP3&gt;<br>
 82: &lt;KP0&gt;<br>
 83: &lt;KP.&gt;<br>
 84: &lt; &gt;<br>
 85: &lt;F11&gt;<br>
 86: &lt;F12&gt;<br>
 87: &lt;KPEnt&gt;<br>
 88: &lt;RCtrl&gt;<br>
 89: &lt;KP/&gt;<br>
 90: &lt;SysRq&gt;<br>
 91: &lt;AltGr&gt;<br>
 92: &lt;Break&gt;<br>
 93: &lt;Home&gt;<br>
 94: &lt;Up&gt;<br>
 95: &lt;PgUp&gt;<br>
 96: &lt;Left&gt;<br>
 97: &lt;Right&gt;<br>
 98: &lt;End&gt;<br>
 99: &lt;Down&gt;<br>
100: &lt;PgDn&gt;<br>
101: &lt;Ins&gt;<br>
102: &lt;Del&gt;<br>
103: &lt;Pause&gt;<br>
104: &lt;LMeta&gt;<br>
105: &lt;RMeta&gt;<br>
106: &lt;Menu&gt;<br>
</code></pre>
</td>
</blockquote></tr>
</table>
How does one know which lines belong to character keys and which lines to function keys?</blockquote>

Well,  the  easiest  way is to use **--export-keymap**, and examine the exported keymap.
Make sure you export in a virtual terminal (Ctrl+Alt+Fn) and not in X as this way there is higher chance of more keys getting exported correctly (don't ask me why).

Basically, **--export-keymap** ouputs 106 lines for 106 keys, even if some of those keys
aren't located on your keyboard. Lines 1, 14, 15, 28, 29, 42, 54-83,  85-106  belong
to  function keys, all other lines (2-13, 16-27, 30-41, 43-53, 84) belong to character keys.

**Line 57 is reserved for Space** and it should always be ' '. Line 84 is reserved for
the  key  just  right  to  left Shift that is present on some international layouts.
Other lines can be quite reliably determined by looking at one exported keymap.  The
keys generally follow the order of their appearance on keyboard, top-to-bottom left-
to-right.

If you create full and completely valid keymap for your particular language, please
upload the contents as a [new issue](http://code.google.com/p/logkeys/issues/) or send it to my e-mail. Thanks!

When arranging your keymap file, you can help yourself with this [keyboard layout tool](http://msdn.microsoft.com/en-us/goglobal/bb964651.aspx) provided by Microsoft.

<wiki:gadget up\_ad\_client="2900001379782823" up\_ad\_slot="3948022173" width="728" height="90" border="0" up\_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" />

# Download #
Here are convenience keymap files provided that you can use. Be aware that using the keymap that matches your keyboard layout doesn't necessarily mean that all keys are covered or that the characters match to those that you type in – the keymap used by your system (loaded by **loadkeys** command for console, and **setxkbmap** command for X) may be set completely arbitrarily. In most cases, though, these should work fine.

(right-click "Save Target As...")

<table width='100%'>
<tr><td width='200px' valign='top'>

<ul><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/en_GB.map'>English (UK)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/fr_CH.map'>French (CH)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/fr.map'>French (FR)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/fr-dvorak-bepo.map'>French (Bepo keyboard)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/de.map'>German</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/hu.map'>Hungarian</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/it.map'>Italian</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/no.map'>Norwegian</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/pt_BR.map'>Portuguese (Brazil)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/pt_PT.map'>Portuguese (Portugal)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/ro.map'>Romanian</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/ru.map'>Russian</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/sk_QWERTY.map'>Slovak (QWERTY)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/sk_QWERTZ.map'>Slovak (QWERTZ)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/sl.map'>Slovene</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/es_AR.map'>Spanish (Argentina)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/es_ES.map'>Spanish (Spain)</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/sv.map'>Swedish</a>
</li><li><a href='http://wiki.logkeys.googlecode.com/git/keymaps/tr.map'>Turkish</a></li></ul>

</td><td width='310px' valign='top'>
<wiki:gadget up_ad_client="2900001379782823" up_ad_slot="9370746681" width="300" height="250" border="0" up_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" /><br>
</td></tr>
</table>

... (upload yours as a [new issue](http://code.google.com/p/logkeys/issues/))

You can easily create keymaps for your layout by taking one example and then modifying it with the help of [keyboard layout tool](http://msdn.microsoft.com/en-us/goglobal/bb964651.aspx) by Microsoft.