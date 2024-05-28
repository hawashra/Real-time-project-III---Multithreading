#include "includes/include.h"
#include "includes/functions.c"
#include "includes/medicine_queue.c"


struct sigaction sa_alrm;
struct sigaction sa_int;

pthread_t *employee_threads;

int liquid_or_pill;
int num_medicine_types;
int prob_liquid_level_correct;
int prob_liquid_color_correct;
int prob_medicine_sealed_correct;
int prob_label_correct;
int prob_pill_count_correct;
int prob_color_size_correct;
int prob_expiry_date_correct;
int production_time;
int max_packs_per_medicine_type;

struct counts* counts_ptr_shm;
int* produced_counts_ptr_shm;

//define the queue 
MedicineQueue *medicine_queue;

sem_t *sem_counts;
sem_t *sem_produced_counts;
sem_t* sem_valid_invalid_counts;

//define the mutex
pthread_mutex_t medicine_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_medicine_queue_not_empty = PTHREAD_COND_INITIALIZER;


//signal handler for the ctrl c 
void exit_handler(int signum){
    makeMedicineQueueEmpty(medicine_queue);
    free(employee_threads);
    closeSharedCounts(counts_ptr_shm);
    closeSharedProducedCounts(produced_counts_ptr_shm);
    
    exit(0);
}

//signal handler produce medicine
void produce_medicine(int signum) {

    UnprocessedMedicine medicine;
    // zero based indexing for the medicine type
    medicine.medicine_type = generateRandomNumber(0, num_medicine_types - 1);
    //fill the boolean values based on the probabilities that are user defined
    medicine.liquid_level_correct = generateRandomNumber(1, 100) <= prob_liquid_level_correct;
    medicine.liquid_color_correct = generateRandomNumber(1, 100) <= prob_liquid_color_correct;
    medicine.medicine_sealed_correct = generateRandomNumber(1, 100) <= prob_medicine_sealed_correct;
    medicine.label_correct = generateRandomNumber(1, 100) <= prob_label_correct;
    medicine.pill_count_correct = generateRandomNumber(1, 100) <= prob_pill_count_correct;
    medicine.pill_color_size_correct = generateRandomNumber(1, 100) <= prob_color_size_correct;
    medicine.expiry_date_correct = generateRandomNumber(1, 100) <= prob_expiry_date_correct;
    medicine.liquid_or_pill = liquid_or_pill;

    sem_wait(sem_produced_counts);
    produced_counts_ptr_shm[medicine.medicine_type]++;
    sem_post(sem_produced_counts);
   
    //fill the probabilites based on
    pthread_mutex_lock(&medicine_queue_mutex);
    enqueueMedicine(medicine_queue, medicine);
    pthread_cond_broadcast(&cond_medicine_queue_not_empty);
    pthread_mutex_unlock(&medicine_queue_mutex);

    if (produced_counts_ptr_shm[medicine.medicine_type] >= max_packs_per_medicine_type)
    {
        printf("Medicine type %d has been produced more than %d times\n", medicine.medicine_type, max_packs_per_medicine_type);
        
        // send a signal to the parent to kill everyone.
        kill(getppid(), SIGINT);

        return;
    }

    //set the alarm again
    alarm(production_time);
}

void* employee_routine_liquid(void* arg)
{
    bool isWorking = false;

    while (1)
    {        
            
        isWorking = false;
        pthread_mutex_lock(&medicine_queue_mutex);
        while (medicine_queue->size == 0)
        {
            pthread_cond_wait(&cond_medicine_queue_not_empty, &medicine_queue_mutex);
        }

        isWorking = true;
        // check if the medicine matches all the requirements
        UnprocessedMedicine medicine = dequeueMedicine(medicine_queue);
        if (medicine.liquid_level_correct && medicine.liquid_color_correct && medicine.medicine_sealed_correct && medicine.label_correct)
        {
            printf("Medicine is correct\n");
            // increment the number of correct medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->valid_liquid_medicine_produced_count++;
            sem_post(sem_counts);
        }
        else
        {
            printf("Medicine is not correct\n");
            // increment the number of incorrect medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->invalid_liquid_medicine_produced_count++;
            sem_post(sem_counts);

        }
        pthread_mutex_unlock(&medicine_queue_mutex);

        sleep(3);
    }

    return (void*) 0;
}


void* employee_routine_pill(void* arg)
{
    while (1)
    {
        pthread_mutex_lock(&medicine_queue_mutex);
        while (medicine_queue->size == 0)
        {

            pthread_cond_wait(&cond_medicine_queue_not_empty, &medicine_queue_mutex);
        }
        // check if the medicine matches all the requirements
        UnprocessedMedicine medicine = dequeueMedicine(medicine_queue);
        if (medicine.pill_count_correct && medicine.pill_color_size_correct && medicine.expiry_date_correct && medicine.label_correct)
        {
            printf("Medicine is correct\n");
            // increment the number of correct medicines in the shared memory
            counts_ptr_shm->valid_pill_medicine_produced_count++;
        }
        else
        {
            printf("Medicine is not correct\n");
            // increment the number of incorrect medicines in the shared memory
            counts_ptr_shm->invalid_pill_medicine_produced_count++;
        }

        pthread_mutex_unlock(&medicine_queue_mutex);

        sleep(3);
    }

    return (void*) 0;
}

int main(int argc, char const *argv[])
{

    srand(getpid());
    
    // open the shared memory
    counts_ptr_shm = openSharedCounts();
    produced_counts_ptr_shm = openSharedProducedCounts();
    // open the semaphores
    sem_counts = sem_open(SEM_COUNTS, 0);
    sem_produced_counts = sem_open(SEM_PRODUCED_COUNTS, 0);

    set_handler(&sa_int, exit_handler, NULL, SIGINT, 0);
    set_handler(&sa_alrm, produce_medicine, NULL, SIGALRM, 0);

    medicine_queue = (MedicineQueue*)malloc(sizeof(MedicineQueue));
    initializeMedicineQueue(medicine_queue);

    int employee_count = atoi (argv[1]);
    liquid_or_pill = atoi (argv[2]);
    num_medicine_types = atoi (argv[3]);
    //we will deal with positive probabilities
    prob_liquid_level_correct = 100 - atoi (argv[4]);
    prob_liquid_color_correct = 100 - atoi (argv[5]);
    prob_medicine_sealed_correct = 100 - atoi (argv[6]);
    prob_label_correct = 100 - atoi (argv[7]);
    prob_pill_count_correct = 100 - atoi (argv[8]);
    prob_color_size_correct = 100 - atoi (argv[9]);
    prob_expiry_date_correct = 100 - atoi (argv[10]);
    production_time = atoi (argv[11]);
    max_packs_per_medicine_type = atoi (argv[12]);
    printf("Hello from production line\n");

    printf("Employee count: %d\n", employee_count);
    printf("Liquid or pill: %d\n", liquid_or_pill);
    printf("Number of medicine types: %d\n", num_medicine_types);
    printf("Probability liquid level correct: %d\n", prob_liquid_level_correct);
    printf("Probability liquid color correct: %d\n", prob_liquid_color_correct);
    printf("Probability medicine sealed correct: %d\n", prob_medicine_sealed_correct);
    printf("Probability label correct: %d\n", prob_label_correct);
    printf("Probability pill count correct: %d\n", prob_pill_count_correct);
    printf("Probability color size correct: %d\n", prob_color_size_correct);
    printf("Probability expiry date correct: %d\n", prob_expiry_date_correct);
    printf("Production time: %d\n", production_time);
    printf("Max packs per medicine type: %d\n", max_packs_per_medicine_type);

    alarm(production_time);
    employee_threads = (pthread_t*)malloc(employee_count * sizeof(pthread_t));
    printf("dbg 8\n");



    for (int i = 0; i < employee_count; i++)
    {
        pthread_create(&employee_threads[i], NULL, liquid_or_pill ? employee_routine_liquid : employee_routine_pill,
        NULL);
    }

    for (int i = 0; i < employee_count; i++)
    {
        pthread_join(employee_threads[i], NULL);
    }

    //innitialize the medicine queue
    sleep(5);
    while(1) {
        //print the size of the queue
        printf("Size of the queue: %d\n", medicine_queue->size);
        //print the rear of the queue
        printf("Rear of the queue: %d\n", medicine_queue->rear->data.medicine_type);
        sleep(5);

    }

    return 0;
}


// busy and non-busy.


/*
    the queue of available threads (not working employees).
*/