#include "app.h"
#include <Windows.h>

int main() {
  if (SetProcessAffinityMask(GetCurrentProcess(), 0x1) == 0) throw;

  App app;
  app.run();
  return 0;
}
