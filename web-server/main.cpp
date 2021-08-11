#include "server.h"
#include <iostream>
int
main()
{
  server server = { "127.0.0.1", 8081 };
  server.start_listen();

  server.accept_incoming();

  return 0;
}
