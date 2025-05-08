#ifndef DEVICE_IPC_H
#define DEVICE_IPC_H
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../code/log/applog.h"

int SocketServerProcess(void);
#endif