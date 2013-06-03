//FMOD sample application
//
//developed for cs335 course CSUB Spring 2013
//by Gordon Griesel
//
//Original source taken from FMOD sample app named: playsound.c
//
//This will allow students to include sounds in their course project.
//
#include "../../api/inc/fmod.h"
//#include "../../api/inc/fmod_errors.h"
#include "../common/wincompat.h"
#include <stdio.h>

#include "fmod.h"

int main(int argc, char *argv[])
{
	FMOD_RESULT result;
	int key, k;
	if (fmod_init()) return 1;
	if (fmod_createsound("../media/drumloop.wav", 0)) return 1;
	if (fmod_createsound("../media/jaguar.wav",   1)) return 1;
	if (fmod_createsound("../media/swish.wav",    2)) return 1;
	fmod_setmode(0,FMOD_LOOP_OFF);
    printf("=============================================\n");
    printf("Sound Example.  FMOD library sound functions. \n");
    printf("=============================================\n");
    printf("\n");
    printf("Press '1' to Play a mono sound using hardware mixing\n");
    printf("Press '2' to Play a mono sound using software mixing\n");
    printf("Press '3' to Play a stereo sound using hardware mixing\n");
    printf("Press 'Esc' to quit\n");
    printf("\n");
	do {
		if (kbhit()) {
			key = getch();
			//all are valid
			//k = key - '0' - 1;
			//k = key - 48 - 1;
			k = key - 0x30 - 1;
			if (k >= 0 && k <= 2) {
				if (fmod_playsound(k)) return 1;
			}
		}
		Sleep(20);
	} while (key != 27);
	fmod_cleanup();
	return 0;
}

