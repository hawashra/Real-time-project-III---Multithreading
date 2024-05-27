#ifndef __INCLUDE_H_
#include "include.h"
#endif

#define BLUE(string) "\033[1;34m" string "\x1b[0m"
#define RED(string) "\033[1;31m" string "\x1b[0m"
#define GREEN(string) "\033[1;32m" string "\x1b[0m"
#define YELLOW(string) "\033[1;33m" string "\x1b[0m"
#define CYAN(string) "\033[1;36m" string "\x1b[0m"
#define MAGENTA(string) "\033[1;35m" string "\x1b[0m"
#define WHITE(string) "\033[1;37m" string "\x1b[0m"
#define BLACK(string) "\033[1;30m" string "\x1b[0m"
#define PURPLE(string) "\033[1;35m" string "\x1b[0m"


void my_pause(double seconds){
    struct timespec ts, rem;
    ts.tv_sec = (time_t) seconds;
    ts.tv_nsec = (long) ((seconds - ts.tv_sec) * 1e9);

    while (nanosleep(&ts, &rem) == -1 && errno == EINTR) {
        ts = rem;
    }
}


// using mode we choose either sa_handler or sa_sigaction
int set_handler(struct sigaction *sa, void (*sa_handler1)(int), void(*sa_sigaction1)(int, siginfo_t*, void* )  , int signum, int mode) {
    if (mode == 0) {
        sa->sa_handler = sa_handler1;
        sa->sa_flags = 0;
    }

    else {
        sa->sa_sigaction = sa_sigaction1;
        sa->sa_flags = SA_SIGINFO;
    }

    sigemptyset(&sa->sa_mask);

    return sigaction(signum, sa, NULL);
}

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}



int openSharedMemory(char* name) {
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); 
    if (shm_fd == -1) {
        perror("Error opening shared memory");

        return EXIT_FAILURE;
    }
    return shm_fd;
}

int ftruncateSharedMemory(int shm_fd, int size) {
    if (ftruncate(shm_fd, size) == -1) {
        perror("Error configuring shared memory size");
        close(shm_fd);
        return EXIT_FAILURE;
    }
    return 0;
}

void* mapSharedMemory(int shm_fd, int size) { //maps a shared memory to the address space of a process
    void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Error mapping shared memory");
        printf("errno: %d\n", errno);
        close(shm_fd);
        return NULL;
    }
    return ptr;
}

void closeSharedMemory(int shm_fd, void* ptr, int size) {
    munmap(ptr, size);
    close(shm_fd);
}


container* openSharedContainers(){
     int shm_fd = openSharedMemory(SHM_CONTAINERS);
    ftruncateSharedMemory(shm_fd, SHM_SIZE);
    // Map the shared memory segment
    container*  ptr = mapSharedMemory(shm_fd, SHM_SIZE);
    return ptr;
}



sharedCount* openSharedCount() {
    int shm_fd = openSharedMemory(SHM_COUNTS);
    ftruncateSharedMemory(shm_fd, sizeof(sharedCount));
    // Map the shared memory segment
    sharedCount*  ptr = mapSharedMemory(shm_fd, sizeof(sharedCount));
    //initialize the shared count
    ptr->numberOfContainers = 0;
    return ptr;
}
