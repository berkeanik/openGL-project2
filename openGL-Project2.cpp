/*********
   CTIS164 - Template Source Program
----------
STUDENT : BERKE ANIK
SECTION : 03
HOMEWORK: HMW-II
----------
PROBLEMS: No problems
----------
ADDITIONAL FEATURES:
- Diamond-shaped special target gives 10 points (star-shaped regular target gives 5 points).
- High score is updated when the highest score is reached.
- Remaining time turns red when 5 seconds left.
- Ufo location is displayed.
*********/
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600
#define TIMER_PERIOD   20 // Period for the timer (50 fps)
#define TIMER_ON        1 // 0:Disable timer, 1:Enable timer
#define D2R 0.01745329252
#define PI  3.14159265358

#define MAX_FIRE 10  // 20 fires at a time.
#define FIRE_RATE 30  // After 8 frames you can throw another one.
#define TARGET_RADIUS 20

// Global variables for Template File
bool spacebar = false;
int  winWidth, winHeight; // Current Window width and height
double remaining = 20;
int score = 0; // score variable
int highestScore = 0; // highest score 
bool pausegame = false; // flag to control pause

// User Defined Types and Variables
typedef struct {
	float x, y;
} point_t;

typedef struct {
	point_t pos;    // Position of the object
	float   angle;  // View angle 
	float   r;
} player_t;

typedef struct {
	point_t pos;
	float angle;
	bool active;
} fire_t;

typedef struct {
	point_t center;
	float radius;
	float speed;
	bool active;
	bool isSpecial; // checking special target
	unsigned char color[3];
} target_t;

player_t player = { {0, -150}, 90, 20 };
fire_t   fire[MAX_FIRE];
target_t targets[MAX_FIRE];
int fire_rate = 0;
int targetTime = 0; // timer for target creation
int targetPeriod = 50; // create a new target every 50 ticks (1 second)

// To draw a filled circle, centered at (x,y) with radius r
void circle(int x, int y, int r) {
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++) {
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

// To display text with variables
void vprint(int x, int y, void* font, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, str[i]);
}

void drawGradient(int x1, int y1, int w, int h, float r, float g, float b) {
	glBegin(GL_QUADS);
	glColor3f(r, g, b);
	glVertex2f(x1, y1);
	glVertex2f(x1 + w, y1);
	glColor3f(r + 0.4, g + 0.4, b + 0.4);
	glVertex2f(x1 + w, y1 - h);
	glVertex2f(x1, y1 - h);
	glEnd();

	glColor3f(0.1, 0.1, 0.1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y1);
	glVertex2f(x1 + w, y1);
	glVertex2f(x1 + w, y1 - h);
	glVertex2f(x1, y1 - h);
	glEnd();
}

void drawPlayer(player_t tp) {
	int x, y, r = 25;
	x = tp.pos.x;
	y = tp.pos.y + 30;

	glLineWidth(2);
	glColor3ub(208, 208, 208);
	circle(x, y, r);

	//body
	glColor3ub(2, 48, 32);
	glBegin(GL_POLYGON);
	glVertex2f(x - r, y);
	glVertex2f(x - r * 1.7, y - r * .7);
	glVertex2f(x - r * 1.3, y - r * 1.3);
	glVertex2f(x + r * 1.3, y - r * 1.3);
	glVertex2f(x + r * 1.7, y - r * .7);
	glVertex2f(x + r, y);
	glEnd();

	//frame 
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x - r, y);
	glVertex2f(x - r * 1.7, y - r * .7);
	glVertex2f(x - r * 1.3, y - r * 1.3);
	glVertex2f(x + r * 1.3, y - r * 1.3);
	glVertex2f(x + r * 1.7, y - r * .7);
	glVertex2f(x + r, y);
	glEnd();

	//bottom
	glColor3ub(0, 0, 0);
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2f(x - r / 4, y - r * 1.3);
	glVertex2f(x - r / 2, y - r * 1.8);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x + r / 4, y - r * 1.3);
	glVertex2f(x + r / 2, y - r * 1.8);
	glEnd();

	//lights
	glColor3ub(255, 191, 0);
	circle(x, y - r / 1.6, r / 2.7);
	circle(x - r / 1.2, y - r / 1.6, r / 3.9);
	circle(x + r / 1.2, y - r / 1.6, r / 3.9);



}

void drawFires() {
	for (int i = 0; i < MAX_FIRE; i++) {
		if (fire[i].active) {
			glColor3f(1, 1, 1);
			circle(fire[i].pos.x, fire[i].pos.y, 6);
		}
	}
}

void drawTargets() {
	for (int i = 0; i < MAX_FIRE; i++) {
		if (targets[i].active) {
			if (targets[i].isSpecial) {
				// draw triangle for special target
				glColor3ubv(targets[i].color);
				glBegin(GL_TRIANGLES);
				glVertex2f(targets[i].center.x, targets[i].center.y + targets[i].radius);
				glVertex2f(targets[i].center.x - targets[i].radius / 2, targets[i].center.y - targets[i].radius / 2);
				glVertex2f(targets[i].center.x + targets[i].radius / 2, targets[i].center.y - targets[i].radius / 2);
				glEnd();
				glBegin(GL_TRIANGLES);
				glVertex2f(targets[i].center.x - targets[i].radius / 2, targets[i].center.y - targets[i].radius / 2);
				glVertex2f(targets[i].center.x + targets[i].radius / 2, targets[i].center.y - targets[i].radius / 2);
				glVertex2f(targets[i].center.x, targets[i].center.y - targets[i].radius);
				glEnd();
			}
			else {
				// draw regular target
				glColor3ubv(targets[i].color);
				int x, y;
				x = targets[i].center.x;
				y = targets[i].center.y;


				glBegin(GL_TRIANGLES);
				glVertex2f(x - 25, y - 12);
				glVertex2f(x + 25, y - 12);
				glVertex2f(x, y + 27);
				glEnd();

				glBegin(GL_TRIANGLES);
				glVertex2f(x - 25, y + 12);
				glVertex2f(x + 25, y + 12);
				glVertex2f(x, y - 27);
				glEnd();
			}
		}
	}
}

// To display onto window using OpenGL commands
void display() {
	// Clear window to black
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	drawGradient(-400, 300, 800, 600, 0.6, 0.1, 0.9); // Sky

	drawPlayer(player);
	drawFires();
	drawTargets();

	glColor3f(0.6, 0, 0.6);
	glRectf(-300, -300, -160, -200);

	glColor3f(0.6, 0, 0.6);
	glRectf(300, -300, 80, -180);


	glColor3f(0.5, 1, 0.5);
	vprint(85, -280, GLUT_BITMAP_8_BY_13, "Ufo location : (%.0f, %.0f)", player.pos.x, player.pos.y);
	glColor3f(0.5, 1, 0.5);
	vprint(-290, -240, GLUT_BITMAP_8_BY_13, "Score: %d", score); // score

	// display highest score
	vprint(-290, -220, GLUT_BITMAP_8_BY_13, "High Score: %d", highestScore);

	if (remaining > 0) {
		if (remaining <= 5) {
			glColor3f(1, 0.0, 0.0);
			vprint(-290, -260, GLUT_BITMAP_8_BY_13, "Time: %.2f sec", remaining);
		}
		else {
			vprint(-290, -260, GLUT_BITMAP_8_BY_13, "Time: %.2f sec", remaining);
		}
		if (pausegame == true) {
			vprint(-40, 0, GLUT_BITMAP_9_BY_15, "GAME PAUSED!");
		}
	}
	else {
		glColor3f(0.0, 0.0, 0.0);
		vprint(-40, 0, GLUT_BITMAP_8_BY_13, "GAME OVER!");
		pausegame = true; // pause the game when remaining time = 0

		// update highest score 
		if (score > highestScore) {
			highestScore = score;
		}
	}

	//key functions
	glColor3f(0.5, 1, 0.5);
	vprint(85, -200, GLUT_BITMAP_8_BY_13, "Keys:");
	vprint(85, -240, GLUT_BITMAP_8_BY_13, "Left Arrow: Move Left");
	vprint(85, -220, GLUT_BITMAP_8_BY_13, "Right Arrow: Move Right");
	vprint(85, -260, GLUT_BITMAP_8_BY_13, "Spacebar: Fire");

	glutSwapBuffers();
}

// In the fire array, check if any fire is available.
// If there is a fire that we can use for firing, return its index.
int findAvailableFire() {
	for (int i = 0; i < MAX_FIRE; i++)
		if (fire[i].active == false) return i;
	return -1;
}

void createTargets() {
	int activeTargetCount = 0;
	for (int i = 0; i < MAX_FIRE; i++) {
		if (!targets[i].active && activeTargetCount < 5) { // max 5 targets
			targets[i].center.x = -300;
			targets[i].center.y = rand() % 250 + 20;
			targets[i].color[1] = rand() % 256;
			targets[i].color[0] = rand() % 256;
			targets[i].color[2] = rand() % 256;
			targets[i].radius = TARGET_RADIUS;
			targets[i].speed = (rand() % 10) / 10.0 + 1.0;
			targets[i].active = true;
			activeTargetCount++;

			// determine special target
			int isSpecial = rand() % 4; // 1 in 4 chance 
			targets[i].isSpecial = (isSpecial == 0);
		}
		else if (targets[i].active) {
			activeTargetCount++;
		}
	}
}

void movePlayer(player_t* tp, float speed) {
	tp->pos.x += speed;

	// if the player goes out, move it back
	if (tp->pos.x > (WINDOW_WIDTH / 2 - 5 - tp->r) || tp->pos.x < (-WINDOW_WIDTH / 2 + 5 + tp->r)) {
		tp->pos.x -= speed;
	}
}

void testCollision() {
	for (int i = 0; i < MAX_FIRE; i++) {
		if (fire[i].active) {
			for (int j = 0; j < MAX_FIRE; j++) {
				if (targets[j].active) {
					float dx = targets[j].center.x - fire[i].pos.x;
					float dy = targets[j].center.y - fire[i].pos.y;
					float d = sqrt(dx * dx + dy * dy);

					if (d <= targets[j].radius) {
						if (targets[j].isSpecial) {
							score += 10;
						}
						else {
							score += 5;
						}
						fire[i].active = false; // deactivate the fire
						targets[j].active = false; // deactivate the target
					}
				}
			}
		}
	}
}


// Key function for ASCII characters like ESC, a,b,c..,A,B,..Z
void onKeyDown(unsigned char key, int x, int y) {
	// Exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = true;
}

void onKeyUp(unsigned char key, int x, int y) {
	// Exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = false;
}

// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
void onSpecialKeyDown(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (!pausegame)
			movePlayer(&player, -8); // moving left
		break;
	case GLUT_KEY_RIGHT:
		if (!pausegame)
			movePlayer(&player, 8); // moving right
		break;
	case GLUT_KEY_F1:
		if (pausegame) {
			pausegame = false; // unpause the game
			if (remaining <= 0) {
				score = 0;
				remaining = 20;
			}
		}
		else {
			pausegame = !pausegame;
		}
		break;
	}
	// Refresh the window by calling display() function
	glutPostRedisplay();
}

// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
void onResize(int w, int h) {
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // Refresh window
}




#if TIMER_ON == 1
void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	if (pausegame)
		return;

	// decrease remaining time if the game is not paused
	if (remaining > 0) {
		remaining -= 0.02;
	}
	else {
		vprint(-40, 0, GLUT_BITMAP_8_BY_13, "GAME OVER!");
		pausegame = true; // pause the game when remaining time = 0

		// update highest score
		if (score > highestScore) {
			highestScore = score;
		}

		return;
	}


	if (spacebar && fire_rate == 0) {
		int availFire = findAvailableFire();
		if (availFire != -1) {
			fire[availFire].pos = player.pos;
			fire[availFire].angle = player.angle;
			fire[availFire].active = true;
			fire_rate = FIRE_RATE;
		}
	}
	if (fire_rate > 0) fire_rate--;

	// Move all fires that are active.
	for (int i = 0; i < MAX_FIRE; i++) {
		if (fire[i].active) {
			fire[i].pos.x += 10 * cos(fire[i].angle * D2R);
			fire[i].pos.y += 10 * sin(fire[i].angle * D2R);
			if (fire[i].pos.x > winWidth / 2 || fire[i].pos.x < -winWidth / 2 || fire[i].pos.y > winHeight / 2 || fire[i].pos.y < -winHeight / 2)
				fire[i].active = false;
		}
	}

	targetTime++;
	if (targetTime >= targetPeriod) {
		createTargets();
		targetTime = 0;
	}

	// Move targets from left to right
	for (int i = 0; i < MAX_FIRE; i++) {
		if (targets[i].active) {
			targets[i].center.x += targets[i].speed;
			if (targets[i].center.x > winWidth / 2) {
				targets[i].active = false; // deactivate targets which go out of bounds
			}
		}
	}

	testCollision();

	// Refresh the window by calling display() function
	glutPostRedisplay(); // display()
}
#endif

void init() {
	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(500, 200);
	glutCreateWindow("Galaxy Defense | [BERKE ANIK]");

	// Window Events
	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	// Keyboard Events
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialFunc(onSpecialKeyDown);

#if TIMER_ON == 1
	// Timer Event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	init();
	glutMainLoop();
	return 0;
}
