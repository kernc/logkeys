AM_CXXFLAGS = -Wall -O3 -DSYS_CONF_DIR=\"$(sysconfdir)\"

bin_PROGRAMS = logkeys llk llkk
logkeys_SOURCES = logkeys.cc
llk_SOURCES = llk.cc
llkk_SOURCES = llkk.cc

install-exec-hook:
	chown root\: $(DESTDIR)$(bindir)/llk
	chmod u+s    $(DESTDIR)$(bindir)/llk
	chown root\: $(DESTDIR)$(bindir)/llkk
	chmod u+s    $(DESTDIR)$(bindir)/llkk
  