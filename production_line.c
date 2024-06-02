#include "includes/include.h"
#include "includes/functions.c"
#include "includes/medicine_queue.c"


struct sigaction sa_alrm;
struct sigaction sa_int;
struct sigaction sa_usr1;
struct sigaction sa_usr2;

pthread_t *employee_threads;
bool* thread_should_exit; /* array of booleans to indicate if a thread should exit. Used to stop the threads when the program is terminated.
or when we move an employee from that production line to another line. */

#define EMPLOYEE_WORK_DELAY 2

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
int threshold_unprocessed_queue_size;
int production_line_index; // the index of the production line in the shared memory that holds the queue sizes.
int current_employee_count;
int max_out_of_spec_bottled_medicine;
int max_out_of_spec_pill_medicine;

struct counts* counts_ptr_shm;
int* produced_counts_ptr_shm;
int* queue_sizes_ptr_shm;


//define the queue 
MedicineQueue *medicine_queue;

sem_t *sem_counts;
sem_t *sem_produced_counts;
sem_t* sem_valid_invalid_counts;
sem_t* sem_queue_sizes;


//define the mutex
pthread_mutex_t medicine_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_medicine_queue_not_empty = PTHREAD_COND_INITIALIZER;


void remove_employee_from_production_line_handler_usr1();
void add_employee_to_production_line_handler_usr2();

//signal handler for the ctrl c 
void exit_handler(int signum) {

    //we will make the thread_should_exit to true for all the threads
    for (int i = 0; i < current_employee_count; i++){
        thread_should_exit[i] = true;
    }
    printf("Exiting production line\n");
    makeMedicineQueueEmpty(medicine_queue);
    assert(employee_threads != NULL);
    free(employee_threads);
    assert(thread_should_exit != NULL);
    free(thread_should_exit);
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

    sem_wait(sem_queue_sizes);
    queue_sizes_ptr_shm[production_line_index]++;
    sem_post(sem_queue_sizes);

    if (produced_counts_ptr_shm[medicine.medicine_type] >= max_packs_per_medicine_type)
    {
        printf("Medicine type %d has been produced more than %d times\n", medicine.medicine_type, max_packs_per_medicine_type);
        
        // send a signal to the parent to kill everyon (end the program)
        kill(getppid(), SIGINT);

        return;
    }

    
    //set the alarm again
    alarm(production_time);
}

void* employee_routine_liquid(void* arg)
{

    int employee_idx = *(int*) arg;
    printf("Employee %d\n", employee_idx);

    while (1)
    {        
        // check if the thread should exit, used mainly when we move an employee from a production line to another
        if (thread_should_exit[employee_idx])
        {
            printf("Exiting employee %d\n", employee_idx);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&medicine_queue_mutex);
        while (medicine_queue->size == 0)
        {
            pthread_cond_wait(&cond_medicine_queue_not_empty, &medicine_queue_mutex);
        }

        // check if the medicine matches all the requirements
        UnprocessedMedicine medicine = dequeueMedicine(medicine_queue);
        sem_wait(sem_queue_sizes);
        queue_sizes_ptr_shm[production_line_index]--;
        sem_post(sem_queue_sizes);


        if (medicine.liquid_level_correct && medicine.liquid_color_correct && medicine.medicine_sealed_correct && medicine.label_correct)
        {
            printf(production_line_index%2 ? CYAN("Medicine is correct") "\n": RED("Medicine is correct") "\n");
            // increment the number of correct medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->valid_liquid_medicine_produced_count++;
            sem_post(sem_counts);
        }
        else
        {
            printf(production_line_index%2 ? CYAN("Medicine is not correct") "\n": RED("Medicine is not correct") "\n");
            // increment the number of incorrect medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->invalid_liquid_medicine_produced_count++;
            
            // check if the liquid out of spec medicines have exceeded the threshold
            if (counts_ptr_shm->invalid_liquid_medicine_produced_count >= max_out_of_spec_bottled_medicine)
            {
                printf("Number of out of spec bottled medicines has exceeded the threshold\n");
                // send a signal to the parent to kill everyone (end the program)
                kill(getppid(), SIGINT);
                return (void*) 0;
            }
          
            sem_post(sem_counts);

        }
        pthread_mutex_unlock(&medicine_queue_mutex);

        my_pause(EMPLOYEE_WORK_DELAY);
    }

    return (void*) 0;
}


void* employee_routine_pill(void* arg)
{
    int employee_idx = *(int*) arg;
    printf("Employee %d\n", employee_idx);
    while (1){
         // check if the thread should exit, used mainly when we move an employee from a production line to another
        if (thread_should_exit[employee_idx])
        {
            printf("Exiting employee %d\n", employee_idx);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&medicine_queue_mutex);
        while (medicine_queue->size == 0)
        {

            pthread_cond_wait(&cond_medicine_queue_not_empty, &medicine_queue_mutex);
        }
        // check if the medicine matches all the requirements
        UnprocessedMedicine medicine = dequeueMedicine(medicine_queue);
        sem_wait(sem_queue_sizes);
        queue_sizes_ptr_shm[production_line_index]--;
        sem_post(sem_queue_sizes);

        if (medicine.pill_count_correct && medicine.pill_color_size_correct && medicine.expiry_date_correct && medicine.label_correct)
        {
            printf(production_line_index%2 ? CYAN("Medicine is correct") "\n": RED("Medicine is correct") "\n");
            // increment the number of correct medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->valid_pill_medicine_produced_count++;
            sem_post(sem_counts);
        }
        else
        {
            printf(production_line_index%2 ? CYAN("Medicine is not correct") "\n": RED("Medicine is not correct") "\n");
            // increment the number of incorrect medicines in the shared memory
            sem_wait(sem_counts);
            counts_ptr_shm->invalid_pill_medicine_produced_count++;
            // check if the pill out of spec medicines have exceeded the threshold
            if (counts_ptr_shm->invalid_pill_medicine_produced_count >= max_out_of_spec_pill_medicine)
            {
                printf("Number of out of spec pill medicines has exceeded the threshold\n");
                // send a signal to the parent to kill everyone (end the program)
                kill(getppid(), SIGINT);
                return (void*) 0;
            }
            sem_post(sem_counts);
        }

        pthread_mutex_unlock(&medicine_queue_mutex);

        my_pause(EMPLOYEE_WORK_DELAY);
    }

    return (void*) 0;
}

int main(int argc, char const *argv[])
{

    srand(getpid());
    
    // open the shared memory
    counts_ptr_shm = openSharedCounts();
    produced_counts_ptr_shm = openSharedProducedCounts();
    queue_sizes_ptr_shm = openSharedQueueSizes();
    // open the semaphores
    sem_counts = sem_open(SEM_COUNTS, 0);
    sem_produced_counts = sem_open(SEM_PRODUCED_COUNTS, 0);
    sem_queue_sizes = sem_open(SEM_QUEUE_SIZES, 0);

    set_handler(&sa_int, exit_handler, NULL, SIGINT, 0);
    set_handler(&sa_alrm, produce_medicine, NULL, SIGALRM, 0);
    set_handler(&sa_usr1, remove_employee_from_production_line_handler_usr1, NULL, SIGUSR1, 0);
    set_handler(&sa_usr2, add_employee_to_production_line_handler_usr2, NULL, SIGUSR2, 0);
    
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
    threshold_unprocessed_queue_size = atoi (argv[13]);
    production_line_index = atoi (argv[14]);
    max_out_of_spec_bottled_medicine = atoi (argv[15]);
    max_out_of_spec_pill_medicine = atoi (argv[16]);

    // printf("Hello from production line\n");
    // printf("Employee count: %d\n", employee_count);
    // printf("Liquid or pill: %d\n", liquid_or_pill);
    // printf("Number of medicine types: %d\n", num_medicine_types);
    // printf("Probability liquid level correct: %d\n", prob_liquid_level_correct);
    // printf("Probability liquid color correct: %d\n", prob_liquid_color_correct);
    // printf("Probability medicine sealed correct: %d\n", prob_medicine_sealed_correct);
    // printf("Probability label correct: %d\n", prob_label_correct);
    // printf("Probability pill count correct: %d\n", prob_pill_count_correct);
    // printf("Probability color size correct: %d\n", prob_color_size_correct);
    // printf("Probability expiry date correct: %d\n", prob_expiry_date_correct);
    // printf("Production time: %d\n", production_time);
    // printf("Max packs per medicine type: %d\n", max_packs_per_medicine_type);


    alarm(production_time);
    // make extra space for employee exchange
    employee_threads = (pthread_t*) malloc(employee_count *2 * sizeof(pthread_t));
    thread_should_exit = (bool*) malloc(employee_count * 2 * sizeof(bool));
    //memset the thread_should_exit to false
    memset(thread_should_exit, 0, employee_count * 2 * sizeof(bool));


    for (int i = 0; i < employee_count; i++)
    {
        int *idx = (int*) malloc(sizeof(int));
        *idx = i;
        pthread_create(&employee_threads[i], NULL, liquid_or_pill ? employee_routine_liquid : employee_routine_pill,
        (void*) idx);
    }

    for (int i = 0; i < employee_count; i++)
    {
        pthread_join(employee_threads[i], NULL);
    }

    return 0;
}

void remove_employee_from_production_line_handler_usr1()
{
    thread_should_exit[current_employee_count - 1] = true;
    // make sure the thread is not doing any work
    //we will cancel the last thread
    //pthread_cancel(employee_threads[current_employee_count - 1]);
    current_employee_count--;
}

void add_employee_to_production_line_handler_usr2()
{
    thread_should_exit[current_employee_count] = false;
    int *idx = (int*) malloc(sizeof(int));
    *idx = current_employee_count;
    pthread_create(&employee_threads[current_employee_count], NULL, liquid_or_pill ? employee_routine_liquid : employee_routine_pill,
    (void*) idx);
    current_employee_count++;
    pthread_join(employee_threads[current_employee_count], NULL);
}