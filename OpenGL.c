#include <GL/glut.h>
#include "includes/include.h"
#include "includes/functions.c"

struct sigaction sa_int;


//------------------------------------------------[OpenGL variables]-------------------------------------------------------------//
void startOpenGl();
void display();
void init();
void reshape(int w, int h);
void timer(int value);
void drawRectangle(int x1, int y1, int x2, int y2, float r, float g, float b);
void drawText_size(int centerX, int centerY, const char *str, int size, float r, float g, float b);
void writeIntegerAtPosition(int number, float x, float y, float r, float g, float b);

// Example data dynamically allocated based on numProductionLines and numMedTypes
int numProductionLines;  // Number of production lines, set at runtime
int numMedTypes; // Number of medicine types, set at runtime

char **productionLineTypes; // Types of production lines

float move_x = 300.0; // Initial x position of the rectangle
float move_speed = 2.0; // Speed of movement
float move_y = 300.0; // Initial y position of the rectangle
int elapsedTime = 0; // Timer to indicate time since the start


// -------------------------------------------------[    Shared Memory    ]-------------------------------------------------------------//

struct counts *counts_ptr_shm;
int *produced_counts_ptr_shm;
int *queue_sizes_ptr_shm;
int *num_employees_ptr_shm;

// -------------------------------------------------[    Signal Handler    ]-------------------------------------------------------------//
void exit_handler(int signum) {
    printf(RED("Exiting OpenGL process") "\n");
    // free resources
    free(productionLineTypes);
    closeSharedCounts(counts_ptr_shm);
    closeSharedProducedCounts(produced_counts_ptr_shm);
    closeSharedQueueSizes(queue_sizes_ptr_shm);
    closeSharedNumEmployees(num_employees_ptr_shm);
    exit(0);
}

//-------------------------------------------------[    Main    ]----------------------------------------------------------------//

int main(int argc, char **argv)
{

    set_handler(&sa_int, exit_handler, NULL, SIGINT, 0);

    printf(GREEN("OpenGL process has started") "\n");
   
    numProductionLines = atoi(argv[1]);
    numMedTypes = atoi(argv[2]);

    counts_ptr_shm = openSharedCounts();  // counts for valid and invalid pills and liquids (as in the struct counts below)

    productionLineTypes = (char **)malloc(numProductionLines * sizeof(char *));
     /*
    struct counts {

        int valid_liquid_medicine_produced_count;
        int valid_pill_medicine_produced_count;
        int invalid_liquid_medicine_produced_count;
        int invalid_pill_medicine_produced_count;
    
    };
    */

    produced_counts_ptr_shm = openSharedProducedCounts(); // produced counts for each medicine type
    queue_sizes_ptr_shm = openSharedQueueSizes(); // queue sizes for each production line (unprocessed medicines) 
    num_employees_ptr_shm = openSharedNumEmployees(); // current number of employees for each production line

    printf(YELLOW("Shared memory segments opened") "\n");

    // Initialize arrays
    for (int i = 0; i < numProductionLines; i++) {
        productionLineTypes[i] = (argv[3][i] % 2 == 1) ? "Pill" : "Liquid"; // Alternate types
    }
  
    glutInit(&argc, argv);
    startOpenGl();
    return 0;
}

//------------------------------------------------[OpenGL functions]-------------------------------------------------------------//

void startOpenGl(){
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(720, 800);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Production Line Visualization");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    init();
    glutMainLoop();
}

//--------------------------------------------------------------------

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 600);
    glMatrixMode(GL_MODELVIEW);
}

//--------------------------------------------------------------------

void init()
{
    glClearColor(0, 0, 1, 1);
}

//--------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw upper section with production lines
    int upperSectionHeight = 300;
    int sectionHeight = upperSectionHeight / numProductionLines;
    for (int i = 0; i < numProductionLines; i++) {
        int y1 = 600 - (i + 1) * sectionHeight;
        int y2 = y1 + sectionHeight;

        float colorOffset = (float)i / (float)numProductionLines;
        drawRectangle(0, y1, 600, y2, 0.2 + colorOffset, 0.4 + colorOffset, 0.6 + colorOffset);

        float textColorR = 1.0 - (0.2 + colorOffset);
        float textColorG = 1.0 - (0.4 + colorOffset);
        float textColorB = 1.0 - (0.6 + colorOffset);

        drawText_size(10, y1 + sectionHeight / 2 + 10, productionLineTypes[i], 15, textColorR, textColorG, textColorB);
        drawText_size(200, y1 + sectionHeight / 2 + 10, "Unprocessed: ", 15, textColorR, textColorG, textColorB);
        writeIntegerAtPosition(queue_sizes_ptr_shm[i], 300, y1 + sectionHeight / 2 + 10, textColorR, textColorG, textColorB);
        drawText_size(400, y1 + sectionHeight / 2 + 10, "Employees: ", 15, textColorR, textColorG, textColorB);
        writeIntegerAtPosition(num_employees_ptr_shm[i], 500, y1 + sectionHeight / 2 + 10, textColorR, textColorG, textColorB);
    }

    // Draw lower section with timer and statistics
    int lowerSectionHeight = 300;
    drawRectangle(0, 0, 600, lowerSectionHeight, 0.6, 0.3, 0.3);

    // Timer
    drawText_size(10, lowerSectionHeight - 40, "Time: ", 15, 0, 0, 0);
    writeIntegerAtPosition(elapsedTime, 80, lowerSectionHeight - 40, 0, 0, 0);

    // Produced Medicine
    for (int i = 0; i < numMedTypes; i++) {
        char label[50];
        sprintf(label, "Produced Medicine %d: ", i + 1);
        drawText_size(10, lowerSectionHeight - 70 - 20 * i, label, 15, 0, 0, 0);
        writeIntegerAtPosition(produced_counts_ptr_shm[i], 200, lowerSectionHeight - 70 - 20 * i, 0, 0, 0);
    }

    // Valid/Invalid Pills  
    drawText_size(300, lowerSectionHeight - 40, "Valid Pills: ", 15, 0, 0, 0);
    writeIntegerAtPosition(counts_ptr_shm->valid_liquid_medicine_produced_count, 450, lowerSectionHeight - 40, 0, 0, 0); 
    drawText_size(300, lowerSectionHeight - 70, "Invalid Pills: ", 15, 0, 0, 0);
    writeIntegerAtPosition(counts_ptr_shm->invalid_liquid_medicine_produced_count, 450, lowerSectionHeight - 70, 0, 0, 0); 

    // Valid/Invalid Liquids
    drawText_size(300, lowerSectionHeight - 100, "Valid Liquids: ", 15, 0, 0, 0);
    writeIntegerAtPosition(counts_ptr_shm->valid_pill_medicine_produced_count, 450, lowerSectionHeight - 100, 0, 0, 0); 
    drawText_size(300, lowerSectionHeight - 130, "Invalid Liquids: ", 15, 0, 0, 0);
    writeIntegerAtPosition(counts_ptr_shm->invalid_pill_medicine_produced_count, 450, lowerSectionHeight - 130, 0, 0, 0); 

    glFlush();
}
//--------------------------------------------------------------------

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1000, timer, 0);

    // Update the x position of the rectangle
    move_x += move_speed;
    if (move_x > 550) // If the rectangle reaches the right side, reset its position
        move_x = -50;

    move_y += 2;
    if (move_y > 550) // If the rectangle reaches the right side, reset its position
        move_y = -50;

    // Update timer
    elapsedTime++;

}

//--------------------------------------------------------------------

void drawRectangle(int x1, int y1, int x2, int y2, float r, float g, float b)
{
    glBegin(GL_POLYGON);
    glColor3f(r, g, b);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

//--------------------------------------------------------------------

void drawText_size(int centerX, int centerY, const char *str, int size, float r, float g, float b)
{
    glColor3f(r, g, b); // Set text color
    glRasterPos2i(centerX, centerY);
    const char *text = str;

    if (size == 18)
    {
        for (int i = 0; i < strlen(text); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
    else if (size == 10)
    {
        for (int i = 0; i < strlen(text); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text[i]);
    }
    else if (size == 15)
    {
        for (int i = 0; i < strlen(text); i++)
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
    }
}

//--------------------------------------------------------------------

void writeIntegerAtPosition(int number, float x, float y, float r, float g, float b)
{
    glColor3f(r, g, b); // Set text color
    glRasterPos2f(x, y);
    char str[10];
    sprintf(str, "%d", number);
    for (int i = 0; str[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}
