CXXFLAGS = -Wall -O3 -DSYS_CONF_DIR=\"$(sysconfdir)\"

bin_PROGRAMS = logkeys llk llkk
logkeys_SOURCES = logkeys.cc
llk_SOURCES = llk.cc
llkk_SOURCES = llkk.cc

install-exec-hook:
	chown root\: $(bindir)/llk
	chmod u+s $(bindir)/llk
	chown root\: $(bindir)/llkk
	chmod u+s $(bindir)/llkk
  