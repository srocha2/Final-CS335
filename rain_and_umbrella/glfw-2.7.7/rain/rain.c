//cs335 - Spring 2013
//program: Rain drops
//author:  Gordon Griesel
//purpose: Class exercise
//
//goal: Use a linked list to process real-time objects
//
//	1. define the linked-list node structure
//	2. create nodes
//	3. add node to end of linked-list
//	4. examine all list nodes
//	5. remove node when needed
//	6. at end, empty the list
//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

//These components can be turned on and off
#define USE_SOUND
#define USE_FONTS
#define USE_UMBRELLA
#define USE_TEXTURES
#define USE_LOG


#ifdef USE_LOG
#include "log.h"
#endif //USE_LOG
#include "defs.h"
#include <GL/glfw.h>
#ifdef USE_SOUND


#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#endif //USE_SOUND

#ifdef USE_FONTS
#include "fonts.h"
#endif //USE_FONTS

//macros
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)

//constants
const float timeslice = 1.0f/60.0f;
const float gravity = -0.2f;
#define ALPHA 1

//prototypes
void init(void);
int InitGL(GLvoid);
void checkkey(int k1, int k2);
void physics(void);
void render(void);
void draw_raindrops(void);
void draw_background(void);
extern GLuint loadBMP(const char *imagepath);
extern GLuint tex_readgl_bmp(char *fileName, int alpha_channel);

//Added Functions
void check_mouse(void);
void draw_explosion(int,int);
void draw_end(void);
void draw_projectile(void);

//global variables and constants
int time_control=1;
int xres=800;
int yres=600;

//Bombs Down
typedef struct t_raindrop {
    int type;
    int linewidth;
    int sound;
    Vec pos;
    Vec lastpos;
    Vec vel;
    Vec maxvel;
    Vec force;
    float length;
    struct t_raindrop *prev;
    struct t_raindrop *next;
} Raindrop;

/*Missles Up
  typedef struct t_missile {
  int type;
  int width;
  int length;
  Vec pos;
  Vec lastpos;
  Vec vel;
  Vec maxvel;
  Vec force;
  struct t_raindrop *prev;
  struct t_raindrop *next;
  } Missile;
  */



Raindrop *ihead=NULL;
void delete_rain(Raindrop *node);
void cleanup_raindrops(void);

int pFire = 0;
typedef struct t_projectile{
    int p_linewidth;
    Vec p_pos;
    Vec p_lastpos;
    Vec p_vel;
    Vec p_maxvel;
    Vec p_force;
    float p_length;
    float p_color[4];
    struct t_projectile *prev;
    struct t_projectile *next;
}Projectile;

Projectile *jhead = NULL;
void delete_projectile(Projectile *node);
void cleanup_projectiles(void);

#ifdef USE_UMBRELLA
#define UMBRELLA_FLAT  0
#define UMBRELLA_ROUND 1
typedef struct t_umbrella {
    int shape;
    Vec pos;
    Vec lastpos;
    float width;
    float width2;
    float radius;
} Umbrella;

Umbrella umbrella;
int deflection=0;
void draw_umbrella(void);

int deflection2=0;
#endif //USE_UMBRELLA

int totrain=0;
int maxrain=0;

#ifdef USE_SOUND
int play_sounds    = 0;
#endif //USE_SOUND

//Added Variables
GLuint Bomb;
GLuint Background;
GLuint Explosion;
GLuint Spaceship;
GLuint Missile;
int score;
int lives;
int size;
int missiles_left;
int explosion_X;
int explosion_Y;

int main(int argc, char **argv)
{


	int choice;
	printf("For New Game Choose '1'\n");
	printf("To Quit Choose '2'\n");
	if(scanf("%i",&choice) == 2)
	{
	 exit(EXIT_FAILURE);
	}
	printf("Choose Difficulty:\n");
	printf("'1' For Beginner\n");
	printf("'2' For Intermidiate\n");
	printf("'3' For Expert\n");
	scanf("%d", &lives);
	
	if(lives ==1){
	  lives = lives*15;
	}
	else if(lives == 2){
	  lives= lives*4;
	}
	if(&lives ==3){
	  lives= (lives*1);
	}

/*	if(scanf("%d", lives) ==1){
	lives == 15;
	}
	else if(scanf("%d", &lives) ==2){
	lives = lives + 8;
	}
	if(scanf("%d", &lives) ==3){
		lives= lives + 2;
	}
*/


    int i, nmodes;
    GLFWvidmode glist[256];
    open_log_file();
    srand((unsigned int)time(NULL));
#ifdef USE_SOUND
    //FMOD_RESULT result;
    if (fmod_init()) return 1;
    if (fmod_createsound("../media/sq1track01.wav", 0)) return 1;
    if (fmod_createsound("../media/drip.wav", 1)) return 1;
    fmod_setmode(0,FMOD_LOOP_NORMAL);
    fmod_playsound(0);
    //fmod_systemupdate();
#endif //USE_SOUND
    //
    glfwInit();
    srand(time(NULL));
    nmodes = glfwGetVideoModes(glist, 100);
    xres = glist[nmodes-1].Width;
    yres = glist[nmodes-1].Height;
    Log("setting window to: %i x %i\n",xres,yres);
    //if (!glfwOpenWindow(xres, yres, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
    if (!glfwOpenWindow(xres,yres,8,8,8,0,32,0,GLFW_FULLSCREEN)) {
	close_log_file();
	glfwTerminate();
	return 0;
    }
    glfwSetWindowTitle("Raindrops in linked list");
    glfwSetWindowPos(0, 0);
    init();
    InitGL();
    glfwSetKeyCallback((GLFWkeyfun)(checkkey));
    //glfwSetMousePosCallback(mouse_move);
    //glfwEnable( GLFW_STICKY_KEYS );
    glfwEnable( GLFW_KEY_REPEAT );
    glfwEnable( GLFW_MOUSE_CURSOR );
    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);
    //texture maps must be enabled to draw fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
   // lives = 20;
    missiles_left = 20;
    size = 100;
    //
    while(1) {
	for (i=0; i<time_control; i++)
	    physics();
	render();
	check_mouse();
	glfwSwapBuffers();
	if (glfwGetKey(GLFW_KEY_ESC)) break;
	if (!glfwGetWindowParam(GLFW_OPENED)) break;
    }

    glfwSetKeyCallback((GLFWkeyfun)NULL);
    glfwSetMousePosCallback((GLFWmouseposfun)NULL);
    close_log_file();
    printf("totrain: %i  maxrain: %i\n",totrain,maxrain);
    glfwTerminate();
#ifdef USE_SOUND
    fmod_cleanup();
#endif //USE_SOUND
#ifdef USE_FONTS
    cleanup_fonts();
#endif //USE_FONTS
    cleanup_raindrops();
    cleanup_projectiles();
    return 0;
}

void checkkey(int k1, int k2)
{
    static int shift=0;

    if(k2 ==GLFW_PRESS){
	if(k1 == ' '){
	    pFire = 1;
	    missiles_left--;
	    return;
	}
    }

    if (k2 == GLFW_PRESS) {
	//some key is being pressed now
	if (k1 == GLFW_KEY_LSHIFT || k1 == GLFW_KEY_RSHIFT) {
	    //it is a shift key
	    shift=1;
	    return;
	}
    }
    if (k2 == GLFW_RELEASE) {
	if (k1 == GLFW_KEY_LSHIFT || k1 == GLFW_KEY_RSHIFT) {
	    //the shift key was released
	    shift=0;
	}
	//don't process any other keys on a release
	return;
    }
#ifdef USE_SOUND
    if (k1 == 'S') {
	play_sounds ^= 1;
	return;
    }
#endif //USE_SOUND

    if (k1 == '`') {
	if (--time_control < 0)
	    time_control = 0;
	return;
    }
    if (k1 == '1') {
	if (++time_control > 32)
	    time_control = 32;
	return;
    }
    if (k1 == 'W') {
	if (shift) {
	    //shrink the umbrella
	    umbrella.width *= (1.0 / 1.05);
	} else {
	    //enlarge the umbrella
	    umbrella.width *= 1.05;
	}
	//half the width
	umbrella.width2 = umbrella.width * 0.5;
	umbrella.radius = (float)umbrella.width2;
	return;
    }
    if (k1 == 'D') {
	deflection ^= 1;
    }
    if (k1 == GLFW_KEY_LEFT)  {
	VecCopy(umbrella.pos, umbrella.lastpos);
	umbrella.pos[0] -= 10.0;
    }
    if (k1 == GLFW_KEY_RIGHT)  {
	VecCopy(umbrella.pos, umbrella.lastpos);
	umbrella.pos[0] += 10.0;
    }

}

void init(void)
{
    umbrella.pos[0] = xres/2; 
    umbrella.pos[1] = 100;
    VecCopy(umbrella.pos, umbrella.lastpos);
    umbrella.width = 200.0;
    umbrella.width2 = umbrella.width * 0.5;
    //umbrella.radius = (float)umbrella.width2;
    umbrella.shape = UMBRELLA_FLAT;
}

int InitGL(GLvoid)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //No Transparency
    //umbrella_texture = loadBMP("umb2.bmp"); 
    //Transparency
    Spaceship = tex_readgl_bmp("spaceship.bmp", ALPHA);
    Bomb = tex_readgl_bmp("bomb.bmp", ALPHA);
    Background = tex_readgl_bmp("space.bmp",ALPHA);
    Explosion = tex_readgl_bmp("explosion.bmp",ALPHA);
    Missile = tex_readgl_bmp("missile.bmp",ALPHA);

    return 1;
}

void render(GLvoid)
{

    //Log("render()...\n");
    glfwGetWindowSize(&xres, &yres);
    glViewport(0, 0, xres, yres);
    //clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode (GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, xres, 0, yres, -1, 1);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    /*if(lives<=0){
      while(1){
      draw_end();
      if (glfwGetKey(GLFW_KEY_ESC)) break;
      }
      }*/

    //Background
    draw_background();

    //Bombs
    draw_raindrops();
    draw_projectile();

    if(size > 0){
	size ++;
	draw_explosion(explosion_X,explosion_Y);
/*#ifdef USE_SOUND
	if(fmod_createsound("../media/MissileSound.wav", 0));
	fmod_setmode(0, FMOD_LOOP_OFF);
	fmod_playsound(0);
#endif */
	if(size >=100)
	    size = 0;
 	}

    glDisable(GL_BLEND);

    draw_umbrella();

    //Draw Text
    Rect r;
    r.left   = xres-140;
    r.bot    = yres-50;
    r.center = 0;
    ggprint12(&r, 20, 0x00ff0000, "Total Missles: %i",totrain);
    ggprint12(&r, 20, 0x0000ff00, "Lives Left: %i\n",lives);
    ggprint12(&r, 20, 0x00ff0000, "Score: %i \n", score);
    ggprint12(&r, 20, 0x00ff0000, "Missiles Left: %i \n", missiles_left);
    ggprint12(&r, 20, 0x00ff0000, "exp_X: %i \n", explosion_X);
    ggprint12(&r, 20, 0x00ff0000, "exp_Y: %i \n", explosion_Y);

    glColor3f(1.0f,1.0f,1.0f);
}


void draw_umbrella(void)
{
    glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
    glPushMatrix();
    glTranslatef(umbrella.pos[0],umbrella.pos[1],umbrella.pos[2]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glBindTexture(GL_TEXTURE_2D,Spaceship);
    glBegin(GL_QUADS);
    float w = umbrella.width2;
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-w, -w);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( w, -w);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( w,  w);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-w,  w);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();
    glColor3f(1.0f,1.0f,1.0f);
}

void draw_projectile(void)
{
    if (jhead) {
	Projectile *p_node = jhead;

/*#ifdef USE_SOUND
	if(fmod_createsound("../media/MissileSound.wav", 0));
	fmod_setmode(0, FMOD_LOOP_OFF);
	fmod_playsound(0);
#endif */

	while(p_node) {
	    glPushMatrix();

	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    //Bind Missile to Projectiles
	    glBindTexture(GL_TEXTURE_2D,Missile);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f,0.0f); glVertex2i(p_node->p_pos[0]-(p_node->p_linewidth/2),p_node->p_pos[1]);
	    glTexCoord2f(0.0f,1.0f); glVertex2i(p_node->p_pos[0]-(p_node->p_linewidth/2),p_node->p_pos[1]+p_node->p_length);
	    glTexCoord2f(1.0f,1.0f); glVertex2i(p_node->p_pos[0]+(p_node->p_linewidth/2),p_node->p_pos[1]+p_node->p_length);
	    glTexCoord2f(1.0f,0.0f); glVertex2i(p_node->p_pos[0]+(p_node->p_linewidth/2),p_node->p_pos[1]);
	    glEnd();
	    glDisable(GL_ALPHA_TEST);
	    glBindTexture(GL_TEXTURE_2D,0);
	    glPopMatrix();
/*
#ifdef USE_SOUND
	fmod_createsound("../media/MissileSound.wav", 0);
	fmod_setmode(0, FMOD_LOOP_OFF);
	fmod_playsound(0);
#endif
*/
	    if (p_node->next==NULL) break;
	    p_node = p_node->next;
	}
    }
}

void draw_raindrops(void)
{
    if (ihead) {
	Raindrop *node = ihead;
	while(node) {
	    glPushMatrix();

	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    //Bind Bomb to Raindrops
	    glBindTexture(GL_TEXTURE_2D, Bomb);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f,0.0f); glVertex2i(node->pos[0]-(node->linewidth/2),node->pos[1]);
	    glTexCoord2f(0.0f,1.0f); glVertex2i(node->pos[0]-(node->linewidth/2),node->pos[1]+node->length);
	    glTexCoord2f(1.0f,1.0f); glVertex2i(node->pos[0]+(node->linewidth/2),node->pos[1]+node->length);
	    glTexCoord2f(1.0f,0.0f); glVertex2i(node->pos[0]+(node->linewidth/2),node->pos[1]);

	    glEnd();
	    glDisable(GL_ALPHA_TEST);
	    glBindTexture(GL_TEXTURE_2D,0);

	    glPopMatrix();

	    if (node->next==NULL) break;
	    node = node->next;
	}
    }
}

void draw_background(void)
{
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D,Background);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f,1.0f); glVertex2i(0,yres);
    glTexCoord2f(1.0f,1.0f); glVertex2i(xres,yres);
    glTexCoord2f(1.0f,0.0f); glVertex2i(xres,0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,0);


}

double VecNormalize(Vec vec) {
    Flt len, tlen;
    Flt xlen = vec[0];
    Flt ylen = vec[1];
    Flt zlen = vec[2];
    len = xlen*xlen + ylen*ylen + zlen*zlen;
    if (len == 0.0) {
	MakeVector(0.0,0.0,1.0,vec);
	return 1.0;
    }
    len = sqrt(len);
    tlen = 1.0 / len;
    vec[0] = xlen * tlen;
    vec[1] = ylen * tlen;
    vec[2] = zlen * tlen;
    return(len);
}

void physics(void)
{
    //Log("physics()...\n");
    if (random(1000) < 1) {
	//create new rain drops...
	Raindrop *node = (Raindrop *)malloc(sizeof(Raindrop));
	if (node == NULL) {
	    Log("error allocating node.\n");
	    exit(EXIT_FAILURE);
	}
	node->prev = node->next = NULL;
	node->sound=0;
	node->pos[0] = rnd() * (float)xres;
	node->pos[1] = rnd() * 100.0f + (float)yres;
	VecCopy(node->pos, node->lastpos);
	node->vel[0] = 0.0f;
	node->vel[1] = 0.0f;

	node->linewidth = 15;
	//larger linewidth = faster speed
	node->maxvel[1] = (float)(node->linewidth*.9);
	node->length = 35;
	//put raindrop into linked list
	node->next = ihead;
	if (ihead) ihead->prev = node;
	ihead = node;
	++totrain;
    }
    //

    if (pFire > 0)
    {
	pFire = 0; // reset shoot condition
	Projectile *p_node = (Projectile *)malloc(sizeof(Projectile));
	if (p_node == NULL) {
	    Log("error allocating node.\n");
	    exit(EXIT_FAILURE);
	}
	p_node->prev = p_node->next = NULL;
	p_node->p_pos[0] = umbrella.pos[0]-6; //Create at umbrella x
	p_node->p_pos[1] = umbrella.pos[1]+30; //Create at umbrella y
	VecCopy(p_node->p_pos, p_node->p_lastpos);
	p_node->p_vel[0] = p_node->p_vel[1] = 0.0f;


	p_node->p_linewidth = 15;
	p_node->p_maxvel[1] = (float)(p_node->p_linewidth*.9);
	p_node->p_length = 30;

	p_node->next = jhead;
	if (jhead) jhead->prev = p_node;
	jhead = p_node;
    }





    //move rain droplets
    if (ihead) {
	Raindrop *node = ihead;
	while(node) {
	    //force is toward the ground
	    node->vel[1] += gravity;
	    VecCopy(node->pos, node->lastpos);
	    node->pos[0] += node->vel[0] * timeslice;
	    node->pos[1] += node->vel[1] * timeslice;
	    if (fabs(node->vel[1]) > node->maxvel[1])
		node->vel[1] *= 0.96;
	    node->vel[0] *= 0.999;
	    //
	    if (node->next == NULL) break;
	    node = node->next;
	}
    }
    //move player-fired projectile
    if (jhead) {
	Projectile *p_node = jhead;
	while(p_node) {
	    p_node->p_vel[1] += gravity + 10; 
	    VecCopy(p_node->p_pos, p_node->p_lastpos);
	    p_node->p_pos[0] += p_node->p_vel[0] * timeslice;
	    p_node->p_pos[1] += p_node->p_vel[1] * timeslice;
	    if (fabs(p_node->p_vel[1]) > p_node->p_maxvel[1])
		p_node->p_vel[1] *= 0.96;
	    p_node->p_vel[0] *= 0.999;
	    if (p_node->next == NULL) break;
	    p_node = p_node->next;

	}
    }


    //check rain droplets
    if (ihead) {
	int r,n=0;
	int x, y;
	Raindrop *savenode;
	Raindrop *node = ihead;
	Projectile *p_savenode;
	Projectile *p_node = jhead;
	while(node) {
	    n++;
#ifdef USE_SOUND
	    if (node->pos[1] < 0.0f) {
		//raindrop hit ground
		if (!node->sound && play_sounds) {
		    //small chance that a sound will play
		    r = random(100);
		    if (r==1) fmod_playsound(0);
		    //if (r==2) fmod_playsound(1);
		    //sound plays once per raindrop
		    node->sound=1;
		}
	    }
#endif //USE_SOUND

	    //collision detection for missle/projectile

	    if (jhead){
		if (node->pos[0] >= (p_node->p_pos[0] - 10) &&
			node->pos[0] <= (p_node->p_pos[0] + 10)) {
		    if (node->lastpos[1] > p_node->p_lastpos[1] ||
			    node->lastpos[1] > p_node->p_pos[1]) {
			if (node->pos[1] <= p_node->p_pos[1] ||
				node->pos[1] <= p_node->p_lastpos[1]) {
			    if (node->linewidth > 1) {

				y=yres-y;
				explosion_X = node->pos[0];
				explosion_Y = node->pos[1];
			    	size = 1;

score++;
#ifdef USE_SOUND
	if(fmod_createsound("../media/explosion2.wav", 0));
	fmod_setmode(0, FMOD_LOOP_OFF);
	fmod_playsound(0);
#endif


					
				//savenode = node->next;
				//delete_rain(node);
				//node = savenode;
				//if (node == NULL) break;
				
				//p_savenode = p_node->next;
				//delete_projectile(p_node);
				//p_node = p_savenode;
				//if (p_node == NULL || node == NULL) break;
				
				node->pos[0] = rnd() * (float)xres;
				node->pos[1] = rnd() * 100.0f + (float)yres;

			    }
			}
		    }
		}
	    }

#ifdef USE_UMBRELLA
	    if (1) {
		//collision detection for raindrop on umbrella
		if (umbrella.shape == UMBRELLA_FLAT) {
		    if (node->pos[0] >= (umbrella.pos[0] - umbrella.width2) &&
			    node->pos[0] <= (umbrella.pos[0] + umbrella.width2)) {
			if (node->lastpos[1] > umbrella.lastpos[1] ||
				node->lastpos[1] > umbrella.pos[1]) {
			    if (node->pos[1] <= umbrella.pos[1] ||
				    node->pos[1] <= umbrella.lastpos[1]) {
				if (node->linewidth > 0) {
				    savenode = node->next;
				    delete_rain(node);
				    node = savenode;
				    if (node == NULL) break;
				}
			    }
			}
		    }
		}
		if (umbrella.shape == UMBRELLA_ROUND) {
		    float d0 = node->pos[0] - umbrella.pos[0];
		    float d1 = node->pos[1] - umbrella.pos[1];
		    float distance = sqrt((d0*d0)+(d1*d1));
		    if (distance <= umbrella.radius && node->pos[1] > umbrella.pos[1]) {
			if (node->linewidth > 0) {
			    if (deflection) {
				//deflect raindrop
				double dot;
				Vec v, up = {0,1,0};
				VecSub(node->pos, umbrella.pos, v);
				VecNormalize(v);
				node->pos[0] = umbrella.pos[0] + v[0] * umbrella.radius;
				node->pos[1] = umbrella.pos[1] + v[1] * umbrella.radius;
				dot = VecDot(v,up);
				dot += 1.0;
				node->vel[0] += v[0] * dot * 10.0;
				node->vel[1] += v[1] * dot * 10.0;
			    } else {
				savenode = node->next;
				delete_rain(node);
				node = savenode;
				if (node == NULL) break;
			    }
			}
		    }
		}


		//VecCopy(umbrella.pos, umbrella.lastpos);
	    }
#endif //USE_UMBRELLA
	    if (node->pos[1] < -20.0f) {
		//rain drop is below the visible area
		savenode = node->next;
		delete_rain(node);
		lives--;
		node = savenode;
		if (node == NULL) break;

	    }
	    if (node->next == NULL) break;
	    node = node->next;
	}
	if (maxrain < n)
	    maxrain = n;
    }
}


void delete_rain(Raindrop *node)
{
    //remove a node from linked list
    if (node->next == NULL && node->prev == NULL) {
	//only one item in list
	free(node);
	ihead = NULL;
	return;
    }
    if (node->next != NULL && node->prev == NULL) {
	//at beginning of list
	node->next->prev = NULL;
	free(node);
	return;
    }
    if (node->next == NULL && node->prev != NULL) {
	//at end of list
	node->prev->next = NULL;
	free(node);
	return;
    }
    if (node->next != NULL && node->prev != NULL) {
	//in middle of list
	node->prev->next = node->next;
	node->next->prev = node->prev;
	free(node);
	return;
    }
    //to do: optimize the code above.
}



void delete_projectile(Projectile *p_node)
{
    if (p_node->next == NULL && p_node->prev == NULL) {
	free(p_node);
	jhead = NULL;
	return;
    }
    if (p_node->next != NULL && p_node->prev == NULL) {
	p_node->next->prev = NULL;
	free(p_node);
	return;
    }
    if (p_node->next == NULL && p_node->prev != NULL) {
	p_node->prev->next = NULL;
	free(p_node);
	return;
    }
    if (p_node->next != NULL && p_node->prev != NULL) {
	p_node->prev->next = p_node->next;
	p_node->next->prev = p_node->prev;
	free(p_node);
	return;
    }
}


void cleanup_raindrops(void)
{
    Raindrop *s;
    while(ihead) {
	s = ihead->next;
	free(ihead);
	ihead = s;
    }
}

void cleanup_projectiles(void)
{
    Projectile *p;
    while(jhead) {
	p = jhead->next;
	free(jhead);
	jhead = p;
    }

}

void check_mouse(void){
    int x,y;
    int lbutton=0;

    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
	glfwGetMousePos(&x,&y);
	y=yres-y;
	explosion_X = x;
	explosion_Y = y;
	size = 1;

    }

}

void draw_explosion(int x, int y){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBindTexture(GL_TEXTURE_2D,Explosion);
    glColor3f(1.0f,1.0f,1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f,0.0f); glVertex2i(x-size,y-size);
    glTexCoord2f(0.0f,1.0f); glVertex2i(x-size,y+size);
    glTexCoord2f(1.0f,1.0f); glVertex2i(x+size,y+size);
    glTexCoord2f(1.0f,0.0f); glVertex2i(x+size,y-size);

    glEnd();

    glDisable(GL_ALPHA_TEST);
    glBindTexture(GL_TEXTURE_2D,0);
}

void draw_end(void){
    glColor3f(1.0f,1.0f,0.0f);
    glBegin(GL_QUADS);
    glVertex2i(0,0);
    glVertex2i(xres,0);
    glVertex2i(xres,yres);
    glVertex2i(0,yres);
    glEnd();
}

