#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

//------------------------------------------------[OpenGL variables]-------------------------------------------------------------//
void startOpenGl();
void display();
void init();
void reshape(int w, int h);
void timer(int value);
void rectangle(int x1, int y1, int x2, int y2, float r, float g, float b);
void drawText_size(int centerX, int centerY, const char *str, int size);
void writeIntegerAtPosition(int number, float x, float y);

// Example data
const int numProductionLines = 3;  // Number of production lines
const char *productionLineTypes[] = {"Pill", "Liquid", "Pill"}; // Types of production lines
int unprocessedMedicine[] = {10, 20, 15}; // Unprocessed medicine for each line
int employees[] = {5, 3, 4}; // Number of employees for each line
int producedMedicine[2] = {0, 0}; // Produced pills and liquids
int validPills = 0, invalidPills = 0; // Processed valid/invalid pills
int validLiquids = 0, invalidLiquids = 0; // Processed valid/invalid liquids

float move_x = 300.0; // Initial x position of the rectangle
float move_speed = 2.0; // Speed of movement
float move_y = 300.0; // Initial y position of the rectangle
int elapsedTime = 0; // Timer to indicate time since the start

//-------------------------------------------------[    Main    ]----------------------------------------------------------------//

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    startOpenGl();
    return 0;
}

//------------------------------------------------[OpenGL functions]-------------------------------------------------------------//

void startOpenGl(){
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
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
    glClearColor(1, 1, 1, 1);
}

//--------------------------------------------------------------------

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw upper section with production lines
    int sectionHeight = 450 / numProductionLines;
    for (int i = 0; i < numProductionLines; i++) {
        int y1 = 600 - (i + 1) * sectionHeight;
        int y2 = y1 + sectionHeight;

        // Draw background for each production line section
        rectangle(0, y1, 600, y2, 0.8, 0.8, 0.8);

        // Draw details
        drawText_size(10, y1 + sectionHeight / 2 + 10, productionLineTypes[i], 15);
        drawText_size(200, y1 + sectionHeight / 2 + 10, "Unprocessed: ", 15);
        writeIntegerAtPosition(unprocessedMedicine[i], 300, y1 + sectionHeight / 2 + 10);
        drawText_size(400, y1 + sectionHeight / 2 + 10, "Employees: ", 15);
        writeIntegerAtPosition(employees[i], 500, y1 + sectionHeight / 2 + 10);
    }

    // Draw lower section with timer and statistics
    int lowerSectionHeight = 150;
    rectangle(0, 0, 600, lowerSectionHeight, 0.9, 0.9, 0.9);

    // Timer
    drawText_size(10, 110, "Time: ", 15);
    writeIntegerAtPosition(elapsedTime, 80, 110);

    // Produced Medicine
    drawText_size(10, 80, "Produced Pills: ", 15);
    writeIntegerAtPosition(producedMedicine[0], 200, 80);
    drawText_size(10, 50, "Produced Liquids: ", 15);
    writeIntegerAtPosition(producedMedicine[1], 200, 50);

    // Valid/Invalid Pills
    drawText_size(300, 80, "Valid Pills: ", 15);
    writeIntegerAtPosition(validPills, 450, 80);
    drawText_size(300, 50, "Invalid Pills: ", 15);
    writeIntegerAtPosition(invalidPills, 450, 50);

    // Valid/Invalid Liquids
    drawText_size(300, 20, "Valid Liquids: ", 15);
    writeIntegerAtPosition(validLiquids, 450, 20);
    drawText_size(300, -10, "Invalid Liquids: ", 15);
    writeIntegerAtPosition(invalidLiquids, 450, -10);

    glFlush();
}

//--------------------------------------------------------------------

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timer, 0);

    // Update the x position of the rectangle
    move_x += move_speed;
    if (move_x > 550) // If the rectangle reaches the right side, reset its position
        move_x = -50;

    move_y += 2;
    if (move_y > 550) // If the rectangle reaches the right side, reset its position
        move_y = -50;

    // Update timer
    elapsedTime++;

    // Simulate production (for demonstration purposes, increment every second)
    if (elapsedTime % 60 == 0) {
        producedMedicine[0]++;
        producedMedicine[1]++;
        validPills++;
        invalidPills++;
        validLiquids++;
        invalidLiquids++;
    }
}

//--------------------------------------------------------------------

void rectangle(int x1, int y1, int x2, int y2, float r, float g, float b)
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

void drawText_size(int centerX, int centerY, const char *str, int size)
{
    glRasterPos2i(centerX, centerY); // Position for Square 1 text
    const char *text1 = str;

    if (size == 18)
    {
        glColor3f(1, 1, 1);
        for (int i = 0; i < strlen(text1); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text1[i]);
    }
    else if (size == 10)
    {
        glColor3f(0, 0, 0);
        for (int i = 0; i < strlen(text1); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text1[i]);
    }
    else if (size == 15)
    {
        glColor3f(0, 0, 0);
        for (int i = 0; i < strlen(text1); i++)
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text1[i]);
    }
}

//--------------------------------------------------------------------

void writeIntegerAtPosition(int number, float x, float y)
{
    glRasterPos2f(x, y);
    char str[10];
    sprintf(str, "%d", number);
    for (int i = 0; str[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}
