#include "../Telegram/SourceFiles/settings.h"
#include <cassert>

int main() {
  cSetAutoStart(true);
  assert(cAutoStart() == true);
  cSetAutoStart(false);
  assert(cAutoStart() == false);
  cSetStartUrl(QString::fromLatin1("test"));
  assert(cStartUrl() == QString::fromLatin1("test"));
  return 0;
}
