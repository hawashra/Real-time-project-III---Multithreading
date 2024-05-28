#include "includes/include.h"
#include "includes/functions.c"

#define QUEUE_SIZE_CHECK_INTERVAL 20

int num_employee_per_production_line;
int num_medicine_types;
int num_production_lines;
int prob_liquid_level_out_of_range;
int prob_liquid_color_mismatch;
int prob_medicine_not_properly_sealed;
int prob_incorrect_label;
int prob_missing_pills;
int prob_incorrect_pill_color_size;
int prob_expiry_date_not_clear;
int max_packs_per_medicine_type;
int max_out_of_spec_bottled_medicine;
int max_out_of_spec_pill_based_medicine;
int max_simulation_run_time_minutes;
int min_production_time;
int max_production_time;
int threshold_unprocessed_queue_size;

struct counts* counts_ptr_shm;
int* produced_counts_ptr_shm;
int* queue_sizes_ptr_shm;

sem_t *sem_counts;
sem_t *sem_produced_counts;
sem_t *sem_valid_invalid_counts;
sem_t* sem_queue_sizes;

int alarm_max_num_calls = 0;

void read_userdefined_data(char* filename);
void print_userdefined_data();
void fork_production_lines(int num);
void initialize_shared_mems_and_sems();
void check_queue_sizes_handler();
pid_t *production_lines_pids;


void exit_handler(int signum){
    printf(" is pressed\n");

    for (int i = 0; i < num_production_lines; i++) {
        kill(production_lines_pids[i], SIGINT);
    }

    // free all dynamically allocated memory, close all semaphores and shared memory
    free(production_lines_pids);
    closeSharedCounts(counts_ptr_shm);
    closeSharedProducedCounts(produced_counts_ptr_shm);
    closeSharedQueueSizes(queue_sizes_ptr_shm);
    sem_close(sem_counts);
    sem_close(sem_produced_counts);
    sem_close(sem_queue_sizes);
    sem_unlink(SEM_COUNTS);
    sem_unlink(SEM_PRODUCED_COUNTS);
    sem_unlink(SEM_QUEUE_SIZES);

    exit(0);
}


//termination handler
struct sigaction sa_int, sa_alrm;

int main(int argc, char *argv[])
{

    srand(time(NULL));
    set_handler(&sa_int,exit_handler,NULL,SIGINT,0);

    read_userdefined_data(argv[1]);
    // print_userdefined_data();

    alarm_max_num_calls = (int) (max_simulation_run_time_minutes * 60 / QUEUE_SIZE_CHECK_INTERVAL);

    initialize_shared_mems_and_sems();
    sleep(2);

    production_lines_pids = (pid_t*)malloc(num_production_lines * sizeof(pid_t));
    fork_production_lines(num_production_lines);

    set_handler(&sa_alrm, check_queue_sizes_handler, NULL, SIGALRM, 0);
    alarm(QUEUE_SIZE_CHECK_INTERVAL);

    sleep(2);

    while (1) {

        printf(BLUE("-----------------Production Line Statistics-----------------") "\n");
        for (int i = 0; i < num_medicine_types; i++) {
            printf(BLUE("Type %d: %d") "\n" , i, produced_counts_ptr_shm[i]);
        }

        printf(GREEN("Valid liquid medicine produced: %d") "\n", counts_ptr_shm->valid_liquid_medicine_produced_count);
        printf(GREEN("Valid pill medicine produced: %d") "\n", counts_ptr_shm->valid_pill_medicine_produced_count);
        printf(RED("Invalid liquid medicine produced: %d") "\n", counts_ptr_shm->invalid_liquid_medicine_produced_count);
        printf(RED("Invalid pill medicine produced: %d") "\n", counts_ptr_shm->invalid_pill_medicine_produced_count);

        printf(BLUE("------------------------------------------------------------") "\n");


        sleep(5);
    }
    


    for (int i = 0; i < num_production_lines; i++) {
        waitpid(production_lines_pids[i], NULL, 0);
    }

    return EXIT_SUCCESS;
}


void read_userdefined_data(char* filename)
{
    // Read user-defined
    
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: Cannot open file user-defined.txt\n");
        exit(1);
    }
    
    if (fscanf(fp, "%*[^:]: %d ", &num_employee_per_production_line) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &num_medicine_types) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &num_production_lines) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_liquid_level_out_of_range) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_liquid_color_mismatch) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_medicine_not_properly_sealed) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_incorrect_label) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_missing_pills) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_incorrect_pill_color_size) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &prob_expiry_date_not_clear) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &max_packs_per_medicine_type) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &max_out_of_spec_bottled_medicine) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &max_out_of_spec_pill_based_medicine) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &max_simulation_run_time_minutes) != 1||
        fscanf(fp, "%*[^:]: %d ", &min_production_time) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &max_production_time) != 1 ||
        fscanf(fp, "%*[^:]: %d ", &threshold_unprocessed_queue_size) != 1){
    
        perror("Error reading user-defined values");
        exit(1);
    }

}

void print_userdefined_data()
{
    printf("Number of employees per production line: %d\n", num_employee_per_production_line);
    printf("Number of medicine types: %d\n", num_medicine_types);
    printf("Number of production lines: %d\n", num_production_lines);
    printf("Probability of liquid level out of range: %d\n", prob_liquid_level_out_of_range);
    printf("Probability of liquid color mismatch: %d\n", prob_liquid_color_mismatch);
    printf("Probability of medicine not properly sealed: %d\n", prob_medicine_not_properly_sealed);
    printf("Probability of incorrect label: %d\n", prob_incorrect_label);
    printf("Probability of missing pills: %d\n", prob_missing_pills);
    printf("Probability of incorrect pill color size: %d\n", prob_incorrect_pill_color_size);
    printf("Probability of expiry date not clear: %d\n", prob_expiry_date_not_clear);
    printf("Maximum packs per medicine type: %d\n", max_packs_per_medicine_type);
    printf("Maximum out of spec bottled medicine: %d\n", max_out_of_spec_bottled_medicine);
    printf("Maximum out of spec pill based medicine: %d\n", max_out_of_spec_pill_based_medicine);
    printf("Maximum simulation run time in minutes: %d\n", max_simulation_run_time_minutes);
    printf("Minimum production time: %d\n", min_production_time);
    printf("Maximum production time: %d\n", max_production_time);
    printf("Threshold unprocessed queue size: %d\n", threshold_unprocessed_queue_size);
}


void fork_production_lines(int num) {

    pid_t pid;
    //generate a random number to determine pill or liquid medecine
    
    for (int i = 0; i  < num; i++) {

        int liquid_or_pill = generateRandomNumber(0, 1);// 0 is liquid, 1 is pill
        int production_time = generateRandomNumber(min_production_time, max_production_time);
        //arguments to be passed to the production line
        char employee_count_arg[5];
        char liquid_or_pill_arg[5];
        char num_medicine_types_arg[5];
        char prob_liquid_level_out_of_range_arg[5];
        char prob_liquid_color_mismatch_arg[5];
        char prob_medicine_not_properly_sealed_arg[5];
        char prob_incorrect_label_arg[5];
        char prob_missing_pills_arg[5];
        char prob_incorrect_pill_color_size_arg[5];
        char prob_expiry_date_not_clear_arg[5];
        char production_time_arg[5];
        char max_packs_per_medicine_type_arg[20];
        char threshold_unprocessed_queue_size_arg[20];
        char production_line_index_arg[5];
        char max_out_of_spec_bottled_medicine_arg[10];
        char max_out_of_spec_pill_based_medicine_arg[10];

        //filling arguments
        sprintf(employee_count_arg, "%d", num_employee_per_production_line);
        sprintf(liquid_or_pill_arg, "%d", liquid_or_pill);
        sprintf(num_medicine_types_arg, "%d", num_medicine_types);
        sprintf(prob_liquid_level_out_of_range_arg, "%d", prob_liquid_level_out_of_range);
        sprintf(prob_liquid_color_mismatch_arg, "%d", prob_liquid_color_mismatch);
        sprintf(prob_medicine_not_properly_sealed_arg, "%d", prob_medicine_not_properly_sealed);
        sprintf(prob_incorrect_label_arg, "%d", prob_incorrect_label);
        sprintf(prob_missing_pills_arg, "%d", prob_missing_pills);
        sprintf(prob_incorrect_pill_color_size_arg, "%d", prob_incorrect_pill_color_size);
        sprintf(prob_expiry_date_not_clear_arg, "%d", prob_expiry_date_not_clear);
        sprintf(production_time_arg, "%d", production_time);
        sprintf(max_packs_per_medicine_type_arg, "%d", max_packs_per_medicine_type);
        sprintf(threshold_unprocessed_queue_size_arg, "%d", threshold_unprocessed_queue_size);
        sprintf(production_line_index_arg, "%d", i);
        sprintf(max_out_of_spec_bottled_medicine_arg, "%d", max_out_of_spec_bottled_medicine);
        sprintf(max_out_of_spec_pill_based_medicine_arg, "%d", max_out_of_spec_pill_based_medicine);

        pid = fork();
        assert(pid >= 0);

        if (pid == 0) {
            
            // Child process
            execlp("./production_line", "./production_line", employee_count_arg,liquid_or_pill_arg,
            num_medicine_types_arg,prob_liquid_level_out_of_range_arg,prob_liquid_color_mismatch_arg,prob_medicine_not_properly_sealed_arg, 
            prob_incorrect_label_arg, prob_missing_pills_arg, prob_incorrect_pill_color_size_arg,
             prob_expiry_date_not_clear_arg,production_time_arg, max_packs_per_medicine_type_arg,
            threshold_unprocessed_queue_size_arg,production_line_index_arg,max_out_of_spec_bottled_medicine_arg,
            max_out_of_spec_pill_based_medicine_arg, NULL);
            exit(0);

        } else {
            production_lines_pids[i] = pid;
        }

    }

}

void initialize_shared_mems_and_sems() {

    counts_ptr_shm = openSharedCounts();
    counts_ptr_shm->valid_liquid_medicine_produced_count = 0;
    counts_ptr_shm->valid_pill_medicine_produced_count = 0;
    counts_ptr_shm->invalid_liquid_medicine_produced_count = 0;
    counts_ptr_shm->invalid_pill_medicine_produced_count = 0;

    produced_counts_ptr_shm = openSharedProducedCounts();
    for (int i = 0; i < num_medicine_types; i++) {
        produced_counts_ptr_shm[i] = 0;
    }

    queue_sizes_ptr_shm = openSharedQueueSizes();
    for (int i = 0; i < num_production_lines; i++) {
        queue_sizes_ptr_shm[i] = 0;
    }

    sem_counts = sem_open(SEM_COUNTS, O_CREAT, 0666, 1);
    sem_produced_counts = sem_open(SEM_PRODUCED_COUNTS, O_CREAT, 0666, 1);
    sem_queue_sizes = sem_open(SEM_QUEUE_SIZES, O_CREAT, 0666, 1);

}

void check_queue_sizes_handler() {

    int max_queue_size = 0;
    int max_queue_size_index = 0;

    int min_queue_size = INT_MAX;
    int min_queue_size_index = 0;

    // reading from the shared memory the queue sizes
    sem_wait(sem_queue_sizes);
    for (int i = 0; i < num_production_lines; i++) {

        if (queue_sizes_ptr_shm[i] > max_queue_size) {
            max_queue_size = queue_sizes_ptr_shm[i];
            max_queue_size_index = i;
        }
        if (queue_sizes_ptr_shm[i] < min_queue_size) {
            min_queue_size = queue_sizes_ptr_shm[i];
            min_queue_size_index = i;
        }

    }
    sem_post(sem_queue_sizes);


    // send a signal to the production line with the maximum queue size if it exceeds the threshold
    // (sending an employee from the least busy production line to the most busy production line)
    if (max_queue_size > threshold_unprocessed_queue_size) {
        kill(production_lines_pids[min_queue_size_index], SIGUSR1);

        // send a signal to the production line with the minimum queue size.
        kill(production_lines_pids[max_queue_size_index], SIGUSR2);
    }

    // the simulation time is up
    if (--alarm_max_num_calls == 0) {
        printf("Simulation time is up\n")
        exit_handler(SIGINT);
    }

    alarm(QUEUE_SIZE_CHECK_INTERVAL);
}