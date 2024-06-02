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
#include <assert.h>
#endif

struct counts {

    int valid_liquid_medicine_produced_count;
    int valid_pill_medicine_produced_count;
    int invalid_liquid_medicine_produced_count;
    int invalid_pill_medicine_produced_count;
};


//some enums
// enum typeOfWorker{C,D,S};
/// @brief Enum for the state of the worker
// enum workerState{SAFE,NOT_SAFE};

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


#define SHM_COUNTS "/shm_counts"
#define SHM_PRODUCED_COUNTS "/shm_produced_counts"
#define SHM_QUEUE_SIZES "/shm_queue_sizes"

#define SEM_COUNTS "/sem_counts"
#define SEM_PRODUCED_COUNTS "/sem_produced_counts"
#define SEM_QUEUE_SIZES "/sem_queue_sizes"

#define SHM_SIZE_COUNTS 32
#define SHM_SIZE_QUEUE_SIZES 32