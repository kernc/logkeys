# logkeys Linux keylogger #


## Important announcement for Arch Linux users ##
logkeys was having an [issue on Arch Linux where it produced an empty log](http://code.google.com/p/logkeys/issues/detail?id=60). The issue is now fixed in the repository (fix found by **bytbox** from the Arch Linux community), so if you're on Arch (or you seem to experience this issue), please [checkout the source from git](http://code.google.com/p/logkeys/source/checkout).

## NEWS: logkeys version 0.1.1a (alpha) released ##
  * fixed 100% CPU issue on x64
  * various bug fixes
  * removed pgrep dependency
  * PID file now in /var/run/
  * other symlink attack vulnerability fixes
  * other security fixes
  * code refactoring
  * remote log uploading via HTTP
  * lkl and lklk are now llk and llkk to avoid confusion
  * llk and llkk are now programs that run logkeys-start.sh/-stop.sh scripts
  * also recognize "HID" USB keyboard devices
  * bug fixes

<wiki:gadget up\_ad\_client="2900001379782823" up\_ad\_slot="5408946622" width="728" height="90" border="0" up\_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" />


---

### What is logkeys? ###
**logkeys is a linux keylogger** (GNU/Linux systems only). It is no more advanced than other available linux keyloggers, but is a bit more up to date, it doesn't unreliably repeat keys and it should never crash your X. All in all, it just seems to work. It relies on event interface of the Linux input subsystem. Once set, it logs all common character and function keys, while also being fully aware of Shift and AltGr key modifiers. **It works with serial as well as USB keyboards**.

### What is a keylogger? ###
Keylogger is a software that quietly monitors keyboard input so as to log any keypresses the user makes. Keyloggers can be used by malicious attackers to sniff out passwords and other sensitive textual information, but often times the user himself (or the corporate branch) wants to monitor his computer unattended (or the employees), reliably storing any unauthorized keyboard activity for later inspection. For example, when you leave your PC just to grab a quick bite from the vending machine, you might want to know if anybody was touching it while you were gone. Or you could use it to monitor your supposedly cheating wife, or young kids while they are surfing the web. You could also use it to obtain statistics of your most pressed keys in order to create your custom Dvorak-style keyboard (I've seen that done). Perhaps you want EVERYTHING you've typed or written in the past months archived for ANY purpose... Uses are limitless.

### Are there alternative Linux keyloggers? ###
There is a plethora of keyloggers for Windows, but not so many for Linux.
On GNU/Linux systems and other reasonable operating systems, keyloggers can be easily implemented with a few lines of shell code. Novice users, however, are usually limited to a narrow set of the following tools: **[lkl](http://sourceforge.net/projects/lkl/)** from 2005, **[uberkey](http://gnu.ethz.ch/linuks.mine.nu/uberkey/)**, which appears dead, **[THC-vlogger](http://freeworld.thc.org/releases.php?q=vlogger)**, made by a renowned group of hackers, and **[PyKeylogger](http://pykeylogger.sourceforge.net/)**. All these tools have their pros and cons. Lkl, for example, sometimes abnormally repeats keys and [its keymap configuration is rather awkward](http://www.google.com/search?q=lkl+keymap) for a range of users. Uberkey, which is just over a hundred lines of code, also often repeats keys and what is worse, [it makes your mouse move abruptly](http://www.google.com/search?q=uberkey+mouse+problem), loosing any sense of control. PyKeylogger, on the other hand, while very feature rich, only works in X environment. Finally, there is vlogger, ...umm..., about which I cannot say anything specifically, only that it is receiving low score all around the web and it only logs shell sessions. There may be other tools, but either way **[logkeys](http://code.google.com/p/logkeys/downloads/)** definitely makes a simple and competitive addition.

<wiki:gadget up\_ad\_client="2900001379782823" up\_ad\_slot="3948022173" width="728" height="90" border="0" up\_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" />

### Does logkeys work with USB keyboards? ###
logkeys Linux keylogger works with serial as well as USB keyboards or similar "HID" devices. However, due to a [certain bug](http://code.google.com/p/logkeys/issues/detail?id=42), a [workaround](http://code.google.com/p/logkeys/wiki/Documentation#Empty_log_file_or_%27Couldn%27t_determine_keyboard_device&) is necessary on some setups.

### So what keyboards logkeys does work with? ###
logkeys supports keyboards like on the image below (courtesy of SEOConsultants.com). These are standard 101 to 105-key PC keyboards with no Asian extensions.<br />
<img src='http://wiki.logkeys.googlecode.com/git/images/keyboard.png' alt='104-key PC keyboard' />

<table width='750'>
<tr><td width='340' valign='top'>
<wiki:gadget up_ad_client="2900001379782823" up_ad_slot="8733006390" width="336" height="280" border="0" up_keywords="logkeys,gnu,linux,keylogger,keyboard,serial,usb,software,open-source" url="http://goo.gl/R5bvK" /><br>
</td><td valign='top'>

<font size='3'>So, you think this linux keylogger is for you?<br>
<br>
<a href='http://code.google.com/p/logkeys/downloads/'>Download logkeys</a> now.<br>
<br>
Or checkout a <a href='http://code.google.com/p/logkeys/source/checkout'>much newer version from SVN</a>.</font>

And before using it, <b>please read the <a href='Documentation.md'>documentation</a></b>! Thanks. :-)<br>
<br>
<hr />

<h3>SFW Distractions</h3>
If you want to <b>improve your programming skills</b>, <b>play</b> some of these <b><a href='http://educative-games.org'>educative programming games</a></b>.<br>
<br>
<b>How did these Google ads get here</b>?<br>
See <a href='http://code.google.com/p/gprojecthosting-adsense/'>Adsense for Google Code Hosting project</a>.<br>
<br>
</td></tr>
</table>

<font size='4'><b>Get <a href='http://www.getfreedomain.name'>Free Domain Name</a></b></font> (no ads, no referals, no hosting, no trick... free-for-all domains [click here](http://www.getfreedomain.name/))



