<br />

<br />

---


# Licence #
logkeys is dual licensed under the terms of either [GNU GPLv3](http://www.gnu.org/licenses/gpl-3.0.html) or later (required by Google), or [WTFPLv2](http://sam.zoy.org/wtfpl/) or later. Pick whichever you prefer!

# Installation #

If you have ever installed a Linux program from source, then you should have no trouble installing logkeys.

If you haven't installed from source yet, it is likely you are missing a C++ compiler installed.
Before proceeding please ensure you have **g++** and "similarly trivial tools" ready.
```
$ sudo apt-get install g++    # to install g++ on a Debian-based OS
```
Proceed with
```
$ tar xvzf logkeys-0.1.1a.tar.gz     # to extract the logkeys archive
 
$ cd logkeys-0.1.1a/build    # move to build directory to build there
$ ../configure               # invoke configure from parent directory
$ make                       # make compiles what it needs to compile
( become superuser now )     # you need root to install in system dir
$ make install               # installs binaries, manuals and scripts
```
If you run on any errors during configure stage, your machine must be in a very poor shape. Before installing please confirm that you have a 2.6 branch of Linux kernel, and standard command line utilities such as **ps**, **grep**, and especially **dumpkeys**.

logkeys relies on **dumpkeys** to output at least half correct keysym bindings. For this to be true, you have to set your console keymap. If you have keyboard correctly set to your language in X, **verify that the same characters appear on a virtual terminal** (Ctrl+Alt+Fn) **also**.

[How to set console keymap?](http://www.google.com/search?q=how+to+set+console+keymap)

logkeys also relies on en\_US.UTF-8 locale being present on the system, or any other language using UTF-8. You can confirm you are using UTF-8 locale, if you say
```bash

$ locale -a
C
...
en_US.utf8
...
$ locale
LANG=en_US.UTF-8
LC_CTYPE="en_US.UTF-8"
...
```
If `locale -a` does not return among others the result en\_US.UTF-8 and if LC\_CTYPE environmental variable doesn't contain UTF-8, then logkeys may not work correctly.

If that is the case use either `locale-gen` or install/reconfigure your distribution's locales package, and there include en\_US.UTF-8.
```
$ apropos locale
```

<wiki:gadget up\_ad\_client="2900001379782823" up\_ad\_slot="3948022173" width="728" height="90" border="0" up\_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" />

# Usage how-to #

logkeys is simple. You can either invoke it directly, by typing full command line, or use the provided scripts. There are two helper programs in this package:

  * bin/llk  , which is intended to start the logkeys daemon, and
  * bin/llkk , which is intended to kill it.

bin/llk runs **_etc/logkeys-start.sh_**, and bin/llkk runs **_etc/logkeys-kill.sh_**.

You can use these two **setuid root** programs (**llk** and **llkk**) for starting and stopping the keylogger quickly and covertly. You can modify the two .sh scripts as you like. As the programs are installed with setuid bit set, the root password need not be provided at their runtime.

Default log file is /var/log/logkeys.log and is not readable by others.

I suggest you first test the program manually with
```
 $ touch test.log
 $ logkeys --start --output test.log
```
and in the other terminal follow it with
```
 $ tail --follow test.log
```
and see if the pressed keys match to those noted. If you use a US keyboard layout, use -u switch. Make sure your terminal character locale is set to UTF-8
```
 $ locale
 LANG=xx_YY.UTF-8
 LC_CTYPE="xx_YY.UTF-8"
 ...
```
or alternatively, you need en\_US.UTF-8 locale available on your system
```
 $ locale -a
 ...
 en_US.UTF-8
 ...
```
otherwise you may only see odd characters (like ꑶ etc.) when pressing character keys.

logkeys acts as a daemon, and you stop the running logger process with
```
 $ logkeys --kill
```
(or bin/llkk provided script).

Before using logkeys, please read the manual page first.
```
 $ man logkeys
```

<wiki:gadget up\_ad\_client="2900001379782823" up\_ad\_slot="3948022173" width="728" height="90" border="0" up\_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" />

## Autorun at system start ##
If you want logkeys to autorun when your OS boots, you have several options.
You can edit _/etc/rc.local_ (or _/etc/rc.d/rc.local_) file and add logkeys execution line before the final `exit 0` call, e.g.
```
#!/bin/sh -e
#
# rc.local
#
logkeys --start --keymap=/home/I/custom_key.map --output=/home/I/custom.log --device=event4
exit 0
```
Alternatively, you can put your custom "logkeys execution line" into _etc/logkeys-start.sh_ file, and then use your desktop's "autorun manager" to run _llk_ program, which will then execute said _logkeys-start.sh_ script without prompting you for root/sudo password.

logkeys will automatically terminate on shutdown.

# Troubleshooting #

## Empty log file or 'Couldn't determine keyboard device' error ##
After you run logkeys successfully, if you open the log file and see only the 'Logging started...' and 'Logging stopped...' tag without any keypress "contents," it is very likely that logkeys got your device id wrong.

This may also apply if you get the following error:
```
  logkeys: Couldn't determine keyboard device. :/
```

<table width='750'>
<tr><td valign='top'>

The solution is to determine the correct event device id, and then run logkeys with <b>--device</b> (-d) switch, specifying that device manually.<br>
<br>
The procedure for manually learning the device id to use is as follows:<br>
<br>
As root, for each existing device <i>eventX</i> in <i>/dev/input/</i>, where X is a number between 0 and 31 inclusively, write:<br>
<pre><code> $ cat /dev/input/eventX<br>
</code></pre>
then type some arbitrary characters. If you <b>see any output</b>, that is the <b>device to be used</b>. If you don't see any output, press Ctrl+C and continue with the next device.<br>
<br>
If this happened to be your issue, please <a href='http://code.google.com/p/logkeys/source/checkout'>checkout the latest version from the repository</a> where keyboard recognition is relatively better implemented. If error persists, <b>please add to bug report <a href='http://code.google.com/p/logkeys/issues/detail?id=42'>here</a></b>, attaching your <i>/proc/bus/input/devices</i> file as well as specifying which was the correct event id. Thanks.<br>
<br>
</td><td width='340px' valign='top'>
<wiki:gadget up_ad_client="2900001379782823" up_ad_slot="8733006390" width="336" height="280" border="0" up_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" /><br>
</td></tr>
</table>

## Logkeys outputs wrong characters ##
It is very likely that you will see only some characters recognized, without any hope for Shift and AltGr working even slightly correct, especially when starting logkeys in X. In that case it is better to switch to virtual terminal, e.g. tty4 (Ctrl+Alt+F4), and there execute:
```
 $ logkeys --export-keymap=my_lang.map
```
Then open _my\_lang.map_ in UTF-8 enabled text editor and manually repair any missing or incorrectly determined mappings. Character keys are defined with two to three space-delimited characters per line (first without modifiers, second with shift, third with AltGr), and function keys are strings of **at most** 7 characters.

Make sure your customized keymap follows the [logkeys keymap format specification](Keymaps#Format.md)!

From then on, execute logkeys with **--keymap** switch, e.g.
```
 $ logkeys --start --keymap my_lang.map
```
Again, see if it now works correctly (and character keys appear correct when you are viewing the log file in editor), and opt to modify _bin/llk_ starter script.

If you create full and completely valid keymap for your particular language,
please [upload it as a new issue](http://code.google.com/p/logkeys/issues/) or send it to me by e-mail. Thanks.
Some languages may already have [keymaps available](http://code.google.com/p/logkeys/wiki/Keymaps#Download).

# Known bugs and limitations #

Please report all found bugs on the [issues tracking page](http://code.google.com/p/logkeys/issues/).

# Planned features (roadmap) #
TODO file provided with release currently holds following demanded features:
  * Add support for sending logs via email.
  * Optionally log title of the focused window.
  * Capture clipboard contents.
  * Add support for mouse events (i.e. on mouse click the focus may have changed).
Depending on the amount of extra free time, these features shall be implemented in the foreseeable future.

If you have time on your hands and the required interest, you are welcome to hack at any of those or completely other features yourself.

