#include "server.h"
#include <iostream>
int
main()
{
  server server = { "0.0.0.0", 8081 };
  server.start_listen();
  server.accept_incoming();
  return 0;
}
