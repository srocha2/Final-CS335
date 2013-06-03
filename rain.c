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
// sdfds
// Hellow
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


//global variables and constants
int time_control=1;
int xres=800;
int yres=600;
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
GLuint umbrella_texture;
int deflection=0;
int show_umbrella  = 0;
void draw_umbrella(void);
#endif //USE_UMBRELLA
//
int totrain=0;
int maxrain=0;
int show_mountains = 1;
int show_rain      = 1;
int show_text      = 0;
#ifdef USE_SOUND
int play_sounds    = 0;
#endif //USE_SOUND

//Added Variables
GLuint Bomb;
GLuint Background;
int score;

int main(int argc, char **argv)
{
	int i, nmodes;
	GLFWvidmode glist[256];
	open_log_file();
	srand((unsigned int)time(NULL));
	#ifdef USE_SOUND
	//FMOD_RESULT result;
	if (fmod_init()) return 1;
	if (fmod_createsound("../media/tick.wav", 0)) return 1;
	if (fmod_createsound("../media/drip.wav", 1)) return 1;
	fmod_setmode(0,FMOD_LOOP_OFF);
	//fmod_playsound(0);
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
	//glfwEnable( GLFW_MOUSE_CURSOR );
	glfwDisable( GLFW_MOUSE_CURSOR );
	//
	#ifdef USE_FONTS
	//glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	//texture maps must be enabled to draw fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
	#endif //USE_FONTS
	//
	while(1) {
		for (i=0; i<time_control; i++)
			physics();
		render();
		score++;
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
	return 0;
}

void checkkey(int k1, int k2)
{
	static int shift=0;
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
	if (k1 == 'M') {
		show_mountains ^= 1;
		return;
	}
	if (k1 == 'R') {
		show_rain ^= 1;
		return;
	}
	#ifdef USE_SOUND
	if (k1 == 'S') {
		play_sounds ^= 1;
		return;
	}
	#endif //USE_SOUND
	if (k1 == 'T') {
		show_text ^= 1;
		return;
	}
	#ifdef USE_UMBRELLA
	if (k1 == 'U') {
		show_umbrella ^= 1;
		return;
	}
	#endif //USE_UMBRELLA
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
	#ifdef USE_UMBRELLA
	if (show_umbrella) {
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
		if (k1 == 'P') {
			//umbrella shape
			if (umbrella.shape == UMBRELLA_FLAT) {
				umbrella.shape = UMBRELLA_ROUND;
				umbrella.radius = (float)umbrella.width2;
			} else {
				umbrella.shape = UMBRELLA_FLAT;
			}
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
		if (k1 == GLFW_KEY_UP)  {
			VecCopy(umbrella.pos, umbrella.lastpos);
			umbrella.pos[1] += 10.0;
		}
		if (k1 == GLFW_KEY_DOWN)  {
			VecCopy(umbrella.pos, umbrella.lastpos);
			umbrella.pos[1] -= 10.0;
		}
	}
	#endif //USE_UMBRELLA
}

void init(void)
{
	#ifdef USE_UMBRELLA
	umbrella.pos[0] = 200.0;
	umbrella.pos[1] = 400.0;
	VecCopy(umbrella.pos, umbrella.lastpos);
	umbrella.width = 300.0;
	umbrella.width2 = umbrella.width * 0.5;
	//umbrella.radius = (float)umbrella.width2;
	umbrella.shape = UMBRELLA_FLAT;
	#endif //USE_UMBRELLA
}

int InitGL(GLvoid)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
	//
	//this is for a texture with no transparency
	//umbrella_texture = loadBMP("umb2.bmp"); 
	//
	//this is for a texture with transparency
	umbrella_texture = tex_readgl_bmp("umb2.bmp", ALPHA);
	Bomb = tex_readgl_bmp("bomb.bmp", ALPHA);
	Background = tex_readgl_bmp("space.bmp",ALPHA);
	//
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
	//
	//Background
	if (show_mountains)
		draw_background();
	if (show_rain)
		draw_raindrops();
	glDisable(GL_BLEND);
	#ifdef USE_UMBRELLA
	if (show_umbrella)
		draw_umbrella();
	#endif //USE_UMBRELLA
	//
	//
	if (show_text) {
		//draw some text
		Rect r;
		r.left   = 1520;
		r.bot    = 1020;
		r.center = 0;
		ggprint12(&r, 16, 0x00ff0000, "<M> Mountains: %s",show_mountains==1?"On":"Off");
		ggprint12(&r, 16, 0x00ff0000, "<R> Rain: %s",show_rain==1?"On":"Off");
		#ifdef USE_UMBRELLA
		ggprint12(&r, 16, 0x00ff0000, "<U> Umbrella: %s",show_umbrella==1?"On":"Off");
		ggprint12(&r, 16, 0x00cc6622, "<P> Shape: %s",umbrella.shape==1?"Round":"Flat");
		ggprint12(&r, 16, 0x00cc6622, "<D> Deflection: %s",deflection==1?"On":"Off");
		#endif //USE_UMBRELLA
		#ifdef USE_SOUND
		ggprint12(&r, 20, 0x00cc6622, "<S> Sound: %s",play_sounds==1?"On":"Off");
		#endif //USE_SOUND
		ggprint12(&r, 16, 0x00aaaa00, "total drops: %i",totrain);
		ggprint12(&r, 16, 0x00aaaa00, "max drops: %i\n",maxrain);
		ggprint12(&r, 16, 0x00aaaa00, "Score: %i \n", score);
	}
}

#ifdef USE_UMBRELLA
void draw_umbrella(void)
{
	if (umbrella.shape == UMBRELLA_FLAT) {
		glColor4f(1.0f, 0.2f, 0.2f, 0.5f);
		glLineWidth(8);
		glBegin(GL_LINES);
			glVertex2f(umbrella.pos[0]-umbrella.width2, umbrella.pos[1]);
			glVertex2f(umbrella.pos[0]+umbrella.width2, umbrella.pos[1]);
		glEnd();
		glLineWidth(1);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glPushMatrix();
		glTranslatef(umbrella.pos[0],umbrella.pos[1],umbrella.pos[2]);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glBindTexture(GL_TEXTURE_2D, umbrella_texture);
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
	}
}
#endif //USE_UMBRELLA

void draw_raindrops(void)
{
	if (ihead) {
		Raindrop *node = ihead;
		while(node) {
			glPushMatrix();
			glTranslated(node->pos[0],node->pos[1],node->pos[2]);

			//Bind Bomb to Raindrops
			glBindTexture(GL_TEXTURE_2D, Bomb);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,0.0f); glVertex2i(node->pos[0]-(node->linewidth/2),node->pos[1]);
				glTexCoord2f(0.0f,1.0f); glVertex2i(node->pos[0]-(node->linewidth/2),node->pos[1]+node->length);
				glTexCoord2f(1.0f,1.0f); glVertex2i(node->pos[0]+(node->linewidth/2),node->pos[1]+node->length);
				glTexCoord2f(1.0f,0.0f); glVertex2i(node->pos[0]+(node->linewidth/2),node->pos[1]);

			glEnd();
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
	if (random(150) < 1) {
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

		node->linewidth = 20;
		//larger linewidth = faster speed
		node->maxvel[1] = (float)(node->linewidth*.9);
		node->length = 40;
		//put raindrop into linked list
		node->next = ihead;
		if (ihead) ihead->prev = node;
		ihead = node;
		++totrain;
	}
	//
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
	//
	//check rain droplets
	if (ihead) {
		int r,n=0;
		Raindrop *savenode;
		Raindrop *node = ihead;
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
			#ifdef USE_UMBRELLA
			if (show_umbrella) {
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

void cleanup_raindrops(void)
{
	Raindrop *s;
	while(ihead) {
		s = ihead->next;
		free(ihead);
		ihead = s;
	}
}



