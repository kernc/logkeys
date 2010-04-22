#include <cstdlib>
#include <unistd.h>

int main() {
  setuid(0);
  exit(system(SYS_CONF_DIR "/logkeys-start.sh"));  // SYS_CONF_DIR defined in CXXFLAGS in Makefile.am
}
