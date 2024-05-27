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

typedef struct {
//the plane will have to drop a package every random amout of time
int numberOfContainers;
int dropTime; //the time to drop a container
int periodOfRefill; //time consumed to refill my containers
int xLocation;
int yLocation;
} cargoPlane;

typedef struct {
    int id;
    int height;
    int weight;
} container;

typedef struct {
    int numberOfContainers;
}sharedCount;


typedef struct{
    // enum typeOfWorker type;
    int energy;
}worker;

typedef struct{
int numberWorkers;
//how much wheat they carry 
int carriage;
worker* workers;
// enum workerState state;

} committee;

//the family struct
typedef struct {
    int starvation;
    int numberOfMembers;
    bool isDead;
}family;

typedef struct {
    int x;
}Message;

//the shared memory names and size
#define SHM_CONTAINERS "/shm_containers"
#define SHM_COUNTS "/shm_counts"
#define SHM_STASH "/shm_stash"
#define SHM_FAMILY "/shm_family"
#define SHM_SIZE 2048

// the semaphores
#define SEM_CONTAINERS "/container_sem"
#define SEM_COUNTS "/shared_count_sem"
#define SEM_STASH "/stash_sem"
#define SEM_FAMILY "/family_sem"

#define SIGCOLLECTOR SIGUSR1
#define SIGDISTRIBUTOR SIGUSR2