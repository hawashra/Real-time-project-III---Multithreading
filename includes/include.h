#ifndef __INCLUDE_H_
#define __INCLUDE_H_

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/mman.h> //shared memory posix
#include <bits/types/sigset_t.h>
#include <semaphore.h> //posix semaphores
#include <sys/msg.h>
#include <pthread.h>
#endif

//some enums
// enum typeOfWorker{C,D,S};
/// @brief Enum for the state of the worker
// enum workerState{SAFE,NOT_SAFE};

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
