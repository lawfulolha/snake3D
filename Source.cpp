#include <stdio.h>
#include <time.h>
#include <iostream>
#include <windows.h>
#include <GL/freeglut.h>
#include <string>
#include <vector> 
#include <mmsystem.h>

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define FOOD_SIZE 7.5
#define SNAKE_SIZE 10
#define OBST_SIZE 10

#define SNAKE_TO_FOOD (FOOD_SIZE+SNAKE_SIZE)/2
#define SNAKE_TO_OBST (SNAKE_SIZE + OBST_SIZE)/2
#define FOOD_TO_OBST (FOOD_SIZE + OBST_SIZE)/2
#define BOTTOM -17.0

#define SOUNDTRACK "sound.wav"

GLint   screenWidth = 1080;
GLint   screenHeight = 640;
GLint   minWidth = 0;
GLint   minHeight = 0;
GLint   maxWidth = 200;
GLint   maxHeight = 200;

//відтворення музики
bool st = false;
int playSoundCount = 0;

//змінення карти
int chosenMap = 1;

//Положення об'єктів відносно глядача
GLfloat viewRotationX = 45.0F;
GLfloat viewRotationY = 0.0F;
GLfloat viewRotationZ = 0.0F;
 
static GLfloat zoom = -290.0f;

enum Map
{
    PLAIN = 1,
    RANDOM = 2,
    BOX = 3,
    LABYRINTH = 4,
};

struct Coordinate
{
    GLdouble x, z;
};

class Object {
public:
    Object() {};
    friend int randomize(int high, int low);
    friend void ManipulateViewAngle();
    friend void Run(int);
    friend void Write(std::string);
    friend GLdouble drandomize(GLdouble high, GLdouble low);
};
void ManipulateViewAngle() {
    glRotatef(viewRotationX, 1.0, 0.0, 0.0);
    glRotatef(viewRotationY, 0.0, 1.0, 0.0);
    glRotatef(viewRotationZ, 0.0, 0.0, 1.0);
}
int randomize(int high, int low)
{
    return (rand() % (high - low)) + low;
}
GLdouble drandomize(GLdouble high, GLdouble low) {
    return low + static_cast<GLdouble> (rand()) / (static_cast <GLdouble> (RAND_MAX / (high - low)));
}
void Write(std::string line) {
    for (int i = 0; i < line.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, line[i]);
}

class Food :public Object {
public:
    Coordinate position= { drandomize( maxWidth - FOOD_SIZE / 2,  minWidth + FOOD_SIZE / 2),
                             drandomize(maxHeight - FOOD_SIZE / 2, minHeight + FOOD_SIZE / 2) }; 
     
    Food() {};

    void draw()
    {
        glPushMatrix();
        ManipulateViewAngle();
        glColor3f(0.9, 0.8, 0.2);
        glTranslatef(position.x, BOTTOM + FOOD_SIZE / 2, position.z);
        glScalef(1, 1, 1);
        glutSolidCube(FOOD_SIZE);
        glPopMatrix();
    }

    void generate(std::vector <Coordinate> bodyPos, std::vector <Coordinate> obstacles = {}) {

        int i;

        bool overflow = true;
        while (overflow) {
            position.x = drandomize(maxWidth - FOOD_SIZE / 2, minWidth + FOOD_SIZE / 2);
            position.z = drandomize(maxHeight - FOOD_SIZE / 2, minHeight + FOOD_SIZE / 2);

            for (i = 0; i < bodyPos.size(); i++) {
                if ((position.x < bodyPos[i].x + SNAKE_TO_FOOD) && (position.x > bodyPos[i].x - SNAKE_TO_FOOD) &&
                    (position.z < bodyPos[i].z + SNAKE_TO_FOOD) && (position.z > bodyPos[i].z - SNAKE_TO_FOOD)) {
                    break;
                }
            }
            if (i >= bodyPos.size()) overflow = false;
        }
        overflow = true;

        if (obstacles.size() > 0) {
            while (overflow) {
                position.x = drandomize(maxWidth - FOOD_SIZE / 2, minWidth + FOOD_SIZE / 2);
                position.z = drandomize(maxHeight - FOOD_SIZE / 2, minHeight + FOOD_SIZE / 2);
                for (i = 0; i < obstacles.size(); i++) {
                    if ((position.x < obstacles[i].x + FOOD_TO_OBST) && (position.x > obstacles[i].x - FOOD_TO_OBST) &&
                        (position.z < obstacles[i].z + FOOD_TO_OBST) && (position.z > obstacles[i].z - FOOD_TO_OBST))
                        break;
                }
                if (i >= obstacles.size()) overflow = false;
            }
        }
    }
};

class Snake :public Object {
public:
    std::vector<Coordinate>bodyPos; 
    Coordinate head = { 35,35 };
    std::vector<Coordinate> buffer = { {0,0},{0,0} };
    
    GLuint  direction = 0; 
    GLuint prevDirection = 0;
    Snake() { 
    };

    void generatePosition(std::vector<Coordinate> obstacles) {
        bool overflow = true; int i;
        while (overflow) {
            head.x = drandomize(maxWidth - SNAKE_SIZE / 2, minWidth + SNAKE_SIZE / 2);
            head.z = drandomize(maxHeight - SNAKE_SIZE / 2, minHeight + SNAKE_SIZE / 2); 
            for (i = 0; i < obstacles.size(); i++) {
                if (( head.x < obstacles[i].x + SNAKE_TO_OBST) && ( head.x > obstacles[i].x - SNAKE_TO_OBST) &&
                    ( head.z < obstacles[i].z + SNAKE_TO_OBST) && ( head.z > obstacles[i].z - SNAKE_TO_OBST)) {
                    break;
                }
            }
            if (i >= obstacles.size()) overflow = false;
        }
    }
    void draw() {
        for (int i = 0; i < bodyPos.size(); i++) {
            GLfloat  delta = fmod(i * 0.016, 1);
            glPushMatrix();
            ManipulateViewAngle();
            glTranslatef(bodyPos[i].x, BOTTOM + SNAKE_SIZE / 2, bodyPos[i].z);
            glColor3f(0.674 - delta, 0.509 , 0.705 + delta);//Color 
            glScalef(1, 1, 1);
            glutSolidCube(SNAKE_SIZE);
            glPopMatrix();
        }
    }
    bool collision(std::vector<Coordinate> obstacles = {}) {
        if (bodyPos.size() > 4) {
            for (int i = 0; i < bodyPos.size(); i++) {
                if ((bodyPos[i].x > head.x - SNAKE_SIZE / 2) &&
                    (bodyPos[i].x < head.x + SNAKE_SIZE / 2) &&
                    (bodyPos[i].z > head.z - SNAKE_SIZE / 2) &&
                    (bodyPos[i].z < head.z + SNAKE_SIZE / 2)) {
                    return true;
                }
            }
        }
        if (obstacles.size() > 0) {
            for (int i = 0; i < obstacles.size(); i++) {
                if ((head.x < obstacles[i].x + SNAKE_TO_OBST - 1) &&
                    (head.x > obstacles[i].x - SNAKE_TO_OBST + 1) &&
                    (head.z < obstacles[i].z + SNAKE_TO_OBST - 1) &&
                    (head.z > obstacles[i].z - SNAKE_TO_OBST + 1)) {
                    return true;
                }
            }
        }
        return false;
    }
};

class Game :public Object {
public:
    GLint   level = 1;
    GLint   score = 0;
    GLboolean  isOver = true;
    GLboolean isPaused = false;
    GLboolean isRunning = false;
    Snake* snake;
    Food* food;

    int map;
    std::vector <Coordinate> obstacles{};
    Game() {}
    Game(int chooseMap = 1) {
        map = chooseMap;
        generateObstacles();
        food = new Food{};
        snake = new Snake{}; 
        snake->generatePosition(obstacles); 
        food->generate(snake->bodyPos, obstacles);
    }
    ~Game() {
        delete food;
        delete snake;
    }
    void pause() {
        isPaused = true;
        snake->prevDirection = snake->direction;
        snake->direction = 0;
    }
    void resume() {
        isPaused = false;
        snake->direction = snake->prevDirection;
    }
    void generateObstacles() {

        if (map == PLAIN) { return; }

        if (map == RANDOM) {
            int obstaclesCount = (rand() % (14)) + 3;
            for (int i = 0; i < obstaclesCount; i++) {
                obstacles.push_back({  drandomize( maxWidth - OBST_SIZE / 2,  minWidth + OBST_SIZE / 2) ,
                                       drandomize(maxHeight - OBST_SIZE / 2, minHeight + OBST_SIZE / 2) });
            }
            return;
        }
        else if (map == BOX) {
            int obstaclesCount = maxWidth / OBST_SIZE;
            for (int i = 0; i < obstaclesCount; i++) {
                obstacles.push_back({ GLdouble(OBST_SIZE / 2 + i * OBST_SIZE), OBST_SIZE / 2 });
                obstacles.push_back({ OBST_SIZE / 2  ,  GLdouble(OBST_SIZE / 2 + i * OBST_SIZE) });
                obstacles.push_back({ GLdouble(maxWidth - OBST_SIZE / 2), GLdouble(OBST_SIZE / 2 + i * OBST_SIZE) });
                obstacles.push_back({ GLdouble(OBST_SIZE / 2 + i * OBST_SIZE), GLdouble(maxHeight - OBST_SIZE / 2) });
            }
            return;
        }
        if (map == LABYRINTH) {
            for (int i = 0; i < maxWidth / OBST_SIZE; i++) {
                obstacles.push_back({ GLdouble(OBST_SIZE / 2 + i * OBST_SIZE), OBST_SIZE / 2 });
                obstacles.push_back({ GLdouble(OBST_SIZE / 2 + i * OBST_SIZE),  GLdouble(maxHeight - OBST_SIZE / 2) });
            }
            for (int i = 0; i < maxWidth / (4 * OBST_SIZE); i++) {
                obstacles.push_back({ OBST_SIZE / 2 , GLdouble(OBST_SIZE / 2 + i * OBST_SIZE) });
                obstacles.push_back({ GLdouble(maxHeight - OBST_SIZE / 2) ,  GLdouble(maxHeight - OBST_SIZE / 2 - i * OBST_SIZE) });
                obstacles.push_back({ GLdouble(3 * maxWidth / 4 - OBST_SIZE / 2) , GLdouble(maxHeight / 2 - 3 * OBST_SIZE / 2 - i * OBST_SIZE) });
                obstacles.push_back({ GLdouble(maxWidth / 4 + OBST_SIZE / 2) , GLdouble(3 * maxHeight / 4 - OBST_SIZE / 2 - i * OBST_SIZE) });
            }
            for (int i = 0; i < maxWidth / (2 * OBST_SIZE); i++) {
                obstacles.push_back({ GLdouble(OBST_SIZE / 2 + i * OBST_SIZE)  ,             GLdouble(maxHeight / 4 - OBST_SIZE / 2) });
                obstacles.push_back({ GLdouble(3 * maxWidth / 4 - OBST_SIZE / 2 - i * OBST_SIZE),GLdouble(maxHeight / 2 - OBST_SIZE / 2) });
                obstacles.push_back({ GLdouble(maxWidth - OBST_SIZE / 2 - i * OBST_SIZE) ,   GLdouble(3 * maxHeight / 4 - OBST_SIZE / 2) });
            }
            return;
        }
    }
    void GameStatus() {
        std::string p;

        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(-80, 5);
        p = "Back to menu level: \"BACKSPACE\"";
        for (int i = 0; i < p.size(); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, p[i]);

        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(-80, -5);
        p = "Reset level: \"ENTER\"";
        for (int i = 0; i < p.size(); i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, p[i]);

        glColor3f(0.35, 0.35, 0.4);
        glRasterPos2f(75, 5);
        p = "MAP: ";
        p += (map == PLAIN ? ("PLAIN") : (map == RANDOM ? ("RANDOM") : (map == BOX ? ("BOX") : ("TUNNEL"))));
        Write(p);

        glColor3f(0.33, 0.33, 0.38);
        glRasterPos2f(75, -5);
        p = "Level: " + std::to_string(level) + " Score: " + std::to_string(score);
        Write(p);
    }

    void drawFloor() {

        glPushMatrix();
        ManipulateViewAngle();
        glPushMatrix();
        glColor3f(0.3, 0.3, 0.3);
        glTranslatef(maxWidth / 2, BOTTOM - 2.5, maxHeight / 2);
        glScalef(maxWidth, 5.0, maxHeight);
        glutSolidCube(1);
        glPopMatrix();

        if (obstacles.size() > 0)
            for (int i = 0; i < obstacles.size(); i++) {
                glPushMatrix();
                ManipulateViewAngle();
                glTranslatef(obstacles[i].x, BOTTOM + OBST_SIZE / 2, obstacles[i].z);
                glColor3f(0.35, 0.35, 0.35);
                glScalef(1, 1, 1);
                glutSolidCube(OBST_SIZE);
                glPopMatrix();
            }

    }
    void Reset() {
        level = 1;
        score = 0;
        map = chosenMap;
        obstacles.clear();
        generateObstacles(); 
        isOver = false; 
        snake->bodyPos.clear();
        snake->generatePosition(obstacles);
        snake->direction = 0;
        snake->bodyPos.push_back({ snake->head.x, snake->head.z });
        food->generate(snake->bodyPos, obstacles);
    }
};


void initLight()
{
    GLfloat ambientColor[] = { 0.4, 0.5, 0.6, 1 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

    GLfloat lightColor0[] = { 0.5 , 0.5 , 0.6 , 1.0 };
    GLfloat lightPos0[] = { 90.0 ,  90 ,  90.0 , 0.9 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

    GLfloat lightColor1[] = { 0.8f, 0.5f, 0.4f, 1.0f };
    GLfloat lightPos1[] = { -90, 90, 90, 0.1 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
}

void Initialize(void)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.65, 0.6, 0.65, 1.0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
}

void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1, 800.0);
}

Game* game;

void Run(int value) {

    game->isRunning = true;

    game->snake->buffer[1].x = game->snake->head.x;
    game->snake->buffer[1].z = game->snake->head.z;

    switch (game->snake->direction) {
    case RIGHT: 
        game->snake->head.x += SNAKE_SIZE / 2;
        if (game->snake->head.x > maxWidth - SNAKE_SIZE / 2) game->snake->head.x = minWidth + SNAKE_SIZE / 2;
        break;
    case LEFT: 
        game->snake->head.x -= SNAKE_SIZE / 2;
        if (game->snake->head.x < minWidth + SNAKE_SIZE / 2) game->snake->head.x = maxWidth - SNAKE_SIZE / 2;
        break;
    case UP: 
        game->snake->head.z += SNAKE_SIZE / 2;
        if (game->snake->head.z > maxHeight - SNAKE_SIZE / 2) game->snake->head.z = minHeight + SNAKE_SIZE / 2;
        break;
    case DOWN: 
        game->snake->head.z -= SNAKE_SIZE / 2;
        if (game->snake->head.z < minHeight + SNAKE_SIZE / 2) game->snake->head.z = maxHeight - SNAKE_SIZE / 2;
        break;
    }
    if (game->snake->collision(game->obstacles)) {
        game->isOver = true;
    }
    if (
        (game->snake->head.x < game->food->position.x + SNAKE_TO_FOOD) &&
        (game->snake->head.x > game->food->position.x - SNAKE_TO_FOOD) &&
        (game->snake->head.z < game->food->position.z + SNAKE_TO_FOOD) &&
        (game->snake->head.z > game->food->position.z - SNAKE_TO_FOOD)) {
        game->score++; {
            game->snake->bodyPos.push_back({0, 0});
            if (game->score % 5 == 0 && game->level < 15)
                game->level++; }
        game->food->generate(game->snake->bodyPos, game->obstacles);


    }
    if(!game->isPaused) 
    for (int i = 0; i < game->snake->bodyPos.size(); i++) {
        game->snake->buffer[0].x = game->snake->buffer[1].x;
        game->snake->buffer[0].z = game->snake->buffer[1].z;
        game->snake->buffer[1].x = game->snake->bodyPos[i].x;
        game->snake->buffer[1].z = game->snake->bodyPos[i].z;
        game->snake->bodyPos[i].x = game->snake->buffer[0].x;
        game->snake->bodyPos[i].z = game->snake->buffer[0].z;
    }

    glutTimerFunc(100 - game->level * 3, Run, 0);
}

void WelcomeScreen(int value) {
    glColor3f(0.65, 0.65, 0.65);
    glRasterPos2f(55, 0);
    Write("SNAKE 3D");

    if (game->map == PLAIN) {
        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(55, -15);
        Write("PLAIN");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -30);
        Write("RANDOM");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -45);
        Write("BOX");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -60);
        Write("LABYRINTH");

    }
    else if (game->map == RANDOM) {

        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -15);
        Write("PLAIN");
        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(55, -30);
        Write("RANDOM");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -45);
        Write("BOX");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -60);
        Write("LABYRINTH");
    }
    else if (game->map == BOX) {
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -15);
        Write("PLAIN");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -30);
        Write("RANDOM");
        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(55, -45);
        Write("BOX");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -60);
        Write("LABYRINTH");
    }
    else if (game->map == LABYRINTH) {
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -15);
        Write("PLAIN");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -30);
        Write("RANDOM");
        glColor3f(0.3, 0.3, 0.4);
        glRasterPos2f(55, -45);
        Write("BOX");
        glColor3f(0.7, 0.7, 0.85);
        glRasterPos2f(55, -60);
        Write("LABYRINTH");
    }
    glColor3f(0.3, 0.3, 0.3);
    glRasterPos2f(55, -75);
    Write("Press \"Enter\" to start");
    glColor3f(0.3, 0.3, 0.3);
    glRasterPos2f(55, -90);
    Write("Press \"Escape\" to quit");
}

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    initLight();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
     
    glTranslatef(-maxWidth/2, maxHeight/2, zoom);
    if (!game->isOver && game->isRunning) {
        game->GameStatus();
        game->drawFloor();
        game->food->draw();
        game->snake->draw();
    }
    else if (game->isOver && game->isRunning) {

        WelcomeScreen(0);
        delete game;
        game = new Game(chosenMap);

    }
    else {
        WelcomeScreen(0);

    }
    glutPostRedisplay();
    glutSwapBuffers();
}



void Special(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_RIGHT:
        if (game->snake->direction != LEFT)
            game->snake->direction = RIGHT;
        break;
    case GLUT_KEY_LEFT:
        if (game->snake->direction != RIGHT)
            game->snake->direction = LEFT;
        break;
    case GLUT_KEY_UP: {
        if (game->isRunning) {
            if (game->snake->direction != UP)
                game->snake->direction = DOWN;
        }
        else if (game->isOver) {
            chosenMap--;
            if (chosenMap < 1) chosenMap = 4;
        }
        break; }
    case GLUT_KEY_DOWN: {
        if (game->isRunning) {
            if (game->snake->direction != DOWN)
                game->snake->direction = UP;
        }
        else if (game->isOver) {
            chosenMap++;
            if (chosenMap > 4) chosenMap = 1;
        }
        break; }
    case GLUT_KEY_SHIFT_R: 
        if (!game->isPaused) game->pause();
        else game->resume();
        break;
    case 8: {
        if (!game->isOver)
            game->isOver = true;
        break; }
    case 49: {
        chosenMap = 1;
        glutPostRedisplay();
        break; }
    case 50: {
        chosenMap = 2;
        glutPostRedisplay();
        break; }
    case 51: {
        chosenMap = 3;
        glutPostRedisplay();
        break; }
    case 52: {
        chosenMap = 4;
        glutPostRedisplay(); }
           break;
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 32: {
        if (playSoundCount % 2 == 0)
            st = PlaySound(TEXT(SOUNDTRACK), NULL, SND_ASYNC);
        else st = PlaySound(NULL, NULL, SND_ASYNC);
        playSoundCount++; }
           break; 
    case 49:
        chosenMap = 1;
        break;
    case 50:
        chosenMap = 2;
        break;
    case 51:
        chosenMap = 3;
        break;
    case 52:
        chosenMap = 4;
        break;
    case 8: {
        if (!game->isOver)
            game->isOver = true;
        break;
    }

    case GLUT_KEY_UP:
        if (game->isOver) {
            chosenMap--;
            if (chosenMap < 1) chosenMap = 4;
        }
        break;
    case GLUT_KEY_DOWN:
        if (game->isOver) {
            chosenMap++;
            if (chosenMap > 4) chosenMap = 1;
        }
        break;
    case 13: game->Reset();
        glutPostRedisplay();
        break;
    case 27:
        exit(0);
        break;
    default:
        break;
    }
}

int main(int argc, char** argv) {

    time_t seconds;
    time(&seconds);
    srand((unsigned int)seconds);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Snake OpenGL");
    glutSpecialFunc(Special);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(Display);
    glutReshapeFunc(resize);
    glEnable(GL_DEPTH_TEST);
    game = new Game(chosenMap);
    Run(0);
    Initialize();
    glutMainLoop();
    delete game;

}
