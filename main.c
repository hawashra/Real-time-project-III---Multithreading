#include "includes/include.h"
#include "includes/functions.c"
//this is for the branch jehad
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

void read_userdefined_data(char* filename);
void print_userdefined_data();
void fork_production_lines(int num);

pid_t *production_lines_pids;

int main(int argc, char *argv[])
{
    read_userdefined_data(argv[1]);
    print_userdefined_data();

    fork_production_lines(num_production_lines);


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
    
    if (fscanf(fp, "%*[^:]: %d ", &num_medicine_types) != 1 ||
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
        fscanf(fp, "%*[^:]: %d ", &max_simulation_run_time_minutes) != 1) {
    
        perror("Error reading user-defined values");
        exit(1);
    }

}

void print_userdefined_data()
{
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
}


void fork_production_lines(int num) {

    pid_t pid;

    for (int i = 0; i  < num; i++) {

        pid = fork();
        assert(pid >= 0);

        if (pid == 0) {
            // Child process
            printf("Child process %d\n", getpid());
            exit(0);
        } else {


        }


    }


}