//cs335 - Spring 2013
//Program: Space Defender
//Author:  Gordon Griesel
//	   Kris Cheesman
//	   Major Ebalo
//	   Hector Aguirre
//	   Ivan Melendez
//	   Servand Rocha
//Purpose: CS335--Software Engineering Final Program 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "log.h"
#include "defs.h"
#include <GL/glfw.h>

#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"

#include "fonts.h"

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
void newGame(void);
void difficulty(void);
void twoPlayer(void);
void gameOver(void);
extern GLuint loadBMP(const char *imagepath);
extern GLuint tex_readgl_bmp(char *fileName, int alpha_channel);

//Added Functions
void draw_explosion(int,int,int);
void draw_projectile(void);
void show_explosion(int, int);

//global variables and constants
int time_control=1;
int xres=800;
int yres=600;

int keys[512];

int players;

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

Raindrop *ihead=NULL;
void delete_rain(Raindrop *node);
void cleanup_raindrops(void);

double pFire[2] = {0,0};
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
    int status;
}Projectile;

#define missleCount 40
Projectile Missile[missleCount];
void delete_projectile(int);
void cleanup_projectiles(void);
typedef struct t_umbrella {
    int shape;
    Vec pos;
    Vec lastpos;
    float width;
    float width2;
    float radius;
} Umbrella;

Umbrella umbrella;
Umbrella umbrella2;

void draw_umbrella(void);
void draw_umbrella2(void);


int totrain=0;
int maxrain=0;
int missileNums = 50000;
int play_sounds    = 0;

//Added Variables
#define explodeCount 40 
GLuint Bomb;
GLuint Background;
GLuint Explosion;
GLuint Spaceship;
GLuint missile;
GLuint NewGame;
GLuint Difficulty;
GLuint GameOver;
GLuint TwoPlayer;
int score;
int lives;
int lvl = 0;
int size[explodeCount];
int missiles_left = missleCount;
int explosion_X[explodeCount];
int explosion_Y[explodeCount];
int chain;
int chain_x=0;
int chain_y=0;
double chain_radius=75;
int loop = 0;
int collision = 0;

int main(int argc, char **argv)
{
    int i, nmodes;
    GLFWvidmode glist[256];
    open_log_file();
    srand((unsigned int)time(NULL));
    //FMOD_RESULT result;
    if (fmod_init()) return 1;
    if (fmod_createsound("../media/sq1track01.wav", 0)) return 1;
    if (fmod_createsound("../media/drip.wav", 1)) return 1;
    fmod_setmode(0,FMOD_LOOP_NORMAL);
    fmod_playsound(0);
 if (fmod_createsound("../media/explosion2.wav", 2)) return 1; //laser fire
    //fmod_systemupdate();
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
    glfwSetKeyCallback((GLFWkeyfun)(checkkey));
    glfwSetWindowTitle("Raindrops in linked list");
    glfwSetWindowPos(0, 0);
    init();
    InitGL();
    glfwSetKeyCallback((GLFWkeyfun)(checkkey));
    glfwEnable( GLFW_KEY_REPEAT );
    glfwEnable( GLFW_MOUSE_CURSOR );
    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);
    //texture maps must be enabled to draw fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    lives = 20;
    //missiles_left = 20;
    //size = 100;
    //
    while(1) {
	for (i=0; i<time_control; i++)
	    physics();
	render();
	glfwSwapBuffers();
	if (glfwGetKey(GLFW_KEY_ESC)) break;
	if (!glfwGetWindowParam(GLFW_OPENED)) break;
    }

    glfwSetKeyCallback((GLFWkeyfun)NULL);
    glfwSetMousePosCallback((GLFWmouseposfun)NULL);
    close_log_file();
    printf("totrain:%i  maxrain: %i\n",totrain,maxrain);
    glfwTerminate();
    fmod_cleanup();
    cleanup_fonts();
    cleanup_raindrops();
    cleanup_projectiles();
    return 0;
}

void newGame(void)
{
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D,NewGame);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f,1.0f); glVertex2i(0,yres);
    glTexCoord2f(1.0f,1.0f); glVertex2i(xres,yres);
    glTexCoord2f(1.0f,0.0f); glVertex2i(xres,0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,0);
}

void difficulty(void)
{
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D,Difficulty);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f,1.0f); glVertex2i(0,yres);
    glTexCoord2f(1.0f,1.0f); glVertex2i(xres,yres);
    glTexCoord2f(1.0f,0.0f); glVertex2i(xres,0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,0);
}
void twoPlayer(void)
{
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D,TwoPlayer);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f,1.0f); glVertex2i(0,yres);
    glTexCoord2f(1.0f,1.0f); glVertex2i(xres,yres);
    glTexCoord2f(1.0f,0.0f); glVertex2i(xres,0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,0);
}

void gameOver(void)
{
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D,GameOver);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f,1.0f); glVertex2i(0,yres);
    glTexCoord2f(1.0f,1.0f); glVertex2i(xres,yres);
    glTexCoord2f(1.0f,0.0f); glVertex2i(xres,0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,0);

}

void checkkey(int k1, int k2)
{
    static int shift=0;
    
    if (k2 == GLFW_PRESS) {
	//some key is being pressed now
	keys[k1] = 1;
	if (k1 == GLFW_KEY_LSHIFT || k1 == GLFW_KEY_RSHIFT) {
	    //it is a shift key
	    shift=1;
	    return;
	}
    }
    if (k2 == GLFW_RELEASE) {
	keys[k1] = 0;
	if (k1 == GLFW_KEY_LSHIFT || k1 == GLFW_KEY_RSHIFT) {
	    //the shift key was released
	    shift=0;
	}
	//don't process any other keys on a release
	return;
    }
   
    if (k1 == 'S') {
	play_sounds ^= 1;
	return;
    }

    if (k1 == '`') {
	if (--time_control < 0)
	    time_control = 0;
	return;
    }
/*    if (k1 == '1') {
	if (++time_control > 32)
	    time_control = 32;
	return;
    }
*/

    if (k1 == 'N') {
        lvl= 1;
}
if (k1 == 'Q') {
        exit(EXIT_FAILURE);
        }
if (k1 == '1'){
        lvl = 2;
	players = 0;
}
if (k1 == '2'){
        lvl = 2;
	players = 0;
}
if (k1 == 'I'){
        lvl = 3;
        lives = 50;
        missileNums = 1000;
        time_control = 1;
        }
if (k1 == 'O'){
        lvl = 3;
        lives = 30;
        missileNums = 500;
        time_control = 2;
        }
if (k1 == 'P'){
        lvl = 3;
        lives = 20;
        missileNums=100;
        time_control=3;
        }




}

void init(void)
{

    int i;
    for (i=0; i<256; i++)
    {
    	keys[i] = 0;
    }
    umbrella.pos[0] = xres/2; 
    umbrella.pos[1] = 100;
    VecCopy(umbrella.pos, umbrella.lastpos);
    umbrella.width = 200.0;
    umbrella.width2 = umbrella.width * 0.5;

    if (players > 0) // Initialize player 2
    {
	umbrella2.pos[0] = xres/2;
	umbrella2.pos[1] = 200;
	VecCopy(umbrella2.pos, umbrella2.lastpos);
	umbrella2.width = 200.0;
	umbrella2.width2 = umbrella2.width * 0.5;
    }
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
    Background = loadBMP("space.bmp");
    Explosion = tex_readgl_bmp("explosion.bmp",ALPHA);
    missile = tex_readgl_bmp("missile.bmp",ALPHA);

    NewGame = loadBMP("NewGameQuit.bmp");
    TwoPlayer= loadBMP("PlayerNum.bmp");
    Difficulty = loadBMP("difficulty.bmp");
    GameOver = loadBMP("GameOver.bmp");

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


	if(lives < -1)
        gameOver();


        if(lvl == 0)
        newGame();

        if(lvl ==1)
        twoPlayer();

        if(lvl ==2)
        difficulty();

        if(lvl == 3 && lives > 0) {

    //Background
    draw_background();

    //Bombs
    draw_raindrops();
    draw_projectile();

    int w;
    int c;
    for (w = 0; w < explodeCount; w++)
    if(size[w] > 0){
	chain = 1;
	size[w] ++;
	draw_explosion(w,explosion_X[w],explosion_Y[w]);

	if(size[w] >=100+rnd()*500)
	{
	    size[w] = 0;
	    chain = 0;
	}
    }

    glDisable(GL_BLEND);

    draw_umbrella();
    if ( players == 1)
	draw_umbrella2();

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
}


void draw_umbrella(void)
{
    glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
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

void draw_umbrella2(void)
{
    glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
    glPushMatrix();
    glTranslatef(umbrella2.pos[0],umbrella2.pos[1],umbrella2.pos[2]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glBindTexture(GL_TEXTURE_2D,Spaceship);
    glBegin(GL_QUADS);
    float w2 = umbrella2.width2;
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-w2, -w2);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( w2, -w2);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( w2,  w2);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-w2,  w2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();
    glColor3f(1.0f,1.0f,1.0f);
}

void draw_projectile(void){
    //Log("draw_projectile\n");
    for(loop=0;loop<missleCount;loop++){
        if(Missile[loop].status==1){
            glPushMatrix();
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0f);
            //Bind Missile to Projectiles
            glBindTexture(GL_TEXTURE_2D,missile);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex2i(Missile[loop].p_pos[0]-(Missile[loop].p_linewidth/2),Missile[loop].p_pos[1]);
            glTexCoord2f(0.0f,1.0f); glVertex2i(Missile[loop].p_pos[0]-(Missile[loop].p_linewidth/2),Missile[loop].p_pos[1]+Missile[loop].p_length);
            glTexCoord2f(1.0f,1.0f); glVertex2i(Missile[loop].p_pos[0]+(Missile[loop].p_linewidth/2),Missile[loop].p_pos[1]+Missile[loop].p_length);
            glTexCoord2f(1.0f,0.0f); glVertex2i(Missile[loop].p_pos[0]+(Missile[loop].p_linewidth/2),Missile[loop].p_pos[1]);
            glEnd();
            glDisable(GL_ALPHA_TEST);
            glBindTexture(GL_TEXTURE_2D,0);
            glPopMatrix();
            //Log("Draw Loop: %i \n",loop);
        }
    }
    //Log("Ended draw_projectile\n");
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
    
    //Keep player in viewable space
    if (umbrella.pos[0] >= (float)xres)
	umbrella.pos[0] -= 2.0;
    if (umbrella.pos[0] <= 0)
	umbrella.pos[0] += 2.0;
    //New updated movement controls
    if (keys[GLFW_KEY_LEFT]==1)  {
	VecCopy(umbrella.pos, umbrella.lastpos);
	umbrella.pos[0] -= 1.0;
    }
    if (keys[GLFW_KEY_RIGHT]==1)  {
	VecCopy(umbrella.pos, umbrella.lastpos);
	umbrella.pos[0] += 1.0;
    }

    if (players > 0) //2nd player
    {
	if (umbrella2.pos[0] >= (float)xres)
	    umbrella2.pos[0] -= 2.0;;
	if (umbrella2.pos[0] <= 0)
	    umbrella2.pos[0] += 2.0;

	if (keys['A']==1)  {
	    VecCopy(umbrella2.pos, umbrella2.lastpos);
	    umbrella2.pos[0] -= 1.0;
	}
	if (keys['D']==1)  {
	    VecCopy(umbrella2.pos, umbrella2.lastpos);
	    umbrella2.pos[0] += 1.0;				    
	}

    }

    //Log("physics()...\n");
    if (random(missileNums) < 1) {
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

	// Set random x velocity for missles
	if (random(20) > 10)
	    node->vel[0] = rnd()*20;
	if (random(20) <= 10)
	    node->vel[0] = -rnd()*20;
	//node->vel[0] = 0.0f;
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

    //if (pFire > 0)
    if (keys[GLFW_KEY_UP] == 1)
    {
	//pFire = 0; // reset shoot condition
	if (glfwGetTime() > pFire[0])
	{
	    pFire[0] = glfwGetTime() + 0.5;
	    for(loop=0;loop<missleCount;loop++){
    		Log("Inside Create Missile For\n");
    		if(Missile[loop].status==0){
    		    Log("Inside Create Missile If\n");
    		    missiles_left--;
    		    Missile[loop].status = 1;
    		    Missile[loop].p_pos[0] = umbrella.pos[0]-6;   //Create at umbrella x
    		    Missile[loop].p_pos[1] = umbrella.pos[1]+30;  //Create at umbrella y
    		    VecCopy(Missile[loop].p_pos,Missile[loop].p_lastpos);
    		    Missile[loop].p_vel[0] = 0.0f;
    		    Missile[loop].p_vel[1] = 0.0f;
    		    Missile[loop].p_linewidth = 20;
    		    Missile[loop].p_maxvel[1] = (float)(Missile[loop].p_linewidth*.9);
    		    Missile[loop].p_length = 35;
    		    Log("Done Creating Missile\n");
    		    break;
		}
    	    }
	}
    }

    if (players > 0)
    {
	if (keys['W'] == 1)
    	{
    	    //pFire = 0; // reset shoot condition
	    if (glfwGetTime() > pFire[1])
    	    {
    		pFire[1] = glfwGetTime() + 0.5;
    		for(loop=0;loop<missleCount;loop++){
    		    Log("Inside Create Missile For\n");
    		    if(Missile[loop].status==0){
    			Log("Inside Create Missile If\n");
    			missiles_left--;
    			Missile[loop].status = 1;
    			Missile[loop].p_pos[0] = umbrella2.pos[0]-6;   //Create at umbrella x
			Missile[loop].p_pos[1] = umbrella2.pos[1]+30;  //Create at umbrella y          
	       		VecCopy(Missile[loop].p_pos,Missile[loop].p_lastpos);
    			Missile[loop].p_vel[0] = 0.0f;
			Missile[loop].p_vel[1] = 0.0f;
    			Missile[loop].p_linewidth = 20;
    			Missile[loop].p_maxvel[1] = (float)(Missile[loop].p_linewidth*.9);
    			Missile[loop].p_length = 35;
    			Log("Done Creating player 2 Missile\n");
    			break;
                }
            }
        }
    }

    }

    //Move Bombs Downward
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

	    if (node->pos[0] < 0 || node->pos[0] > (float)xres)
		node->vel[0] = node->vel[0] * (-1);

	    //
	    if (node->next == NULL) break;
	    node = node->next;
	}
    }
    //move player-fired projectile upward

    for(loop=0;loop<missleCount;loop++){
        if(Missile[loop].status==1){
            Missile[loop].p_vel[1] += gravity + 3;
            VecCopy(Missile[loop].p_pos, Missile[loop].p_lastpos);
            Missile[loop].p_pos[0] += Missile[loop].p_vel[0] * timeslice;
            Missile[loop].p_pos[1] += Missile[loop].p_vel[1] * timeslice;
            if (fabs(Missile[loop].p_vel[1]) > Missile[loop].p_maxvel[1])
                Missile[loop].p_vel[1] *= 0.96;
            Missile[loop].p_vel[0] *= 0.999;
            if(Missile[loop].p_pos[1]>yres){
                Missile[loop].status=0;
                missiles_left++;
            }

        }
    }

    //check rain droplets
    if (ihead) {
	int r,n=0;
	int x, y;
	Raindrop *savenode;
	Raindrop *node = ihead;
	//Projectile *p_savenode;
	//Projectile *p_node = jhead;
	while(node) {
	    n++;
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

	    
	    //collision detection for missle/projectile
	    for(loop = 0; loop<missleCount;loop++){
		if(Missile[loop].status == 1){
		    if (node->pos[0] >= (Missile[loop].p_pos[0] - 10)
			    && node->pos[0] <= (Missile[loop].p_pos[0] + 10)) {
                        if (node->lastpos[1] > Missile[loop].p_lastpos[1] 
				|| node->lastpos[1] > Missile[loop].p_pos[1]) {
                            if (node->pos[1] <= Missile[loop].p_pos[1] 
				    || node->pos[1] <= Missile[loop].p_lastpos[1]) {
                                //Collision Detected
				show_explosion(node->pos[0], node->pos[1]);
                                Log("Start to clear\n");

                                savenode = node->next;
                                delete_rain(node);
                                node = savenode;

                                Log("Calling delete_projectile");
                                delete_projectile(loop);
                                collision=1;
                                break; //Done checking missile

                            }
                        }
                    }
                }
                //Log("Finished with collision detection\n");
            }
            if(node == NULL) break;

            if(collision == 1){
                collision = 0;
                break;
            }

	    
	    //Chain nearby explosions
	    if (chain == 1) //if an explosion is active
	    {
		if (node->pos[0] >= (chain_x-chain_radius) &&
			node->pos[0] <= (chain_x+chain_radius)) {
		    if (node->lastpos[1] > (chain_y-chain_radius) ||
			    node->lastpos[1] > (chain_y-chain_radius)) {
			if (node->pos[1] <= (chain_y+chain_radius)||
				node->pos[1] <= (chain_y+chain_radius)) {
			    if (node->linewidth > 1) {
				show_explosion(node->pos[0], node->pos[1]);
				score += 100;

				savenode = node->next;
				delete_rain(node);
				node = savenode;
				if (node ==NULL) break;

			    }
			}
		    }
		}
	    }
	     
	    if (node->pos[0] >= (umbrella.pos[0] - umbrella.width2) &&
		    node->pos[0] <= (umbrella.pos[0] + umbrella.width2)) {
		if (node->lastpos[1] > umbrella.lastpos[1] ||
			node->lastpos[1] > umbrella.pos[1]) {
		    if (node->pos[1] <= umbrella.pos[1] ||
			    node->pos[1] <= umbrella.lastpos[1]) {
			if (node->linewidth > 0) {
			    show_explosion(node->pos[0], node->pos[1]);
			    savenode = node->next;
			    delete_rain(node);
			    node = savenode;
			    if (node == NULL) break;
			}
		    }
		}

		if (players > 0) //If player 2 active
		{
			if (node->pos[0] >= (umbrella2.pos[0] - umbrella2.width2) &&
				node->pos[0] <= (umbrella2.pos[0] + umbrella2.width2)) {
			    if (node->lastpos[1] > umbrella2.lastpos[1] ||
				    node->lastpos[1] > umbrella2.pos[1]) {
				if (node->pos[1] <= umbrella2.pos[1] ||
					node->pos[1] <= umbrella2.lastpos[1]) {
				    if (node->linewidth > 0) {
					show_explosion(node->pos[0], node->pos[1]);
					savenode = node->next;
					delete_rain(node);
					node = savenode;
					if (node == NULL) break;
				    }
				}
			    }
			}
		}
	    }

	    if (node->pos[1] < -20.0f) {
		//rain drop is below the visible area
		show_explosion(node->pos[0], node->pos[1]+30);
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
	ihead = node->next;
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

void delete_projectile(int val){
    Log("Doing delete_projectile:%i\n",val);

    Missile[val].status = 0;
    missiles_left++;

    Log("Done delete_projectile4\n");
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

void cleanup_projectiles(void){
    Log("Starting cleanup_projectile\n");
    for(loop=0;loop<missleCount;loop++){
	Missile[loop].status = 0;
    }
    Log("Finished cleanup_projectile\n");
}

void show_explosion(int x, int y)
{
    int w;
    fmod_playsound(2);

    for (w = 0; w < explodeCount; w++)
    {
    	if (size[w] == 0)
	    	{
	    	    explosion_X[w] = x;
	    	    explosion_Y[w] = y;
	    	    size[w] = 1;

		    chain_x = x;
		    chain_y = y;
	    	    return;
	    	}
    }
}

void draw_explosion(int w,int x, int y){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBindTexture(GL_TEXTURE_2D,Explosion);
    glColor3f(1.0f,rnd()*1.0f,rnd()*1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f,0.0f); glVertex2i(x-size[w],y-size[w]);
    glTexCoord2f(0.0f,1.0f); glVertex2i(x-size[w],y+size[w]);
    glTexCoord2f(1.0f,1.0f); glVertex2i(x+size[w],y+size[w]);
    glTexCoord2f(1.0f,0.0f); glVertex2i(x+size[w],y-size[w]);

    glEnd();

    glDisable(GL_ALPHA_TEST);
    glBindTexture(GL_TEXTURE_2D,0);
}

