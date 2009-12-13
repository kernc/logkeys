CXXFLAGS = -Wall -O3

bin_PROGRAMS = logkeys
logkeys_SOURCES = logkeys.cc

install-exec-hook:
	chown root $(bindir)/logkeys
	chmod u+s $(bindir)/logkeys
