#ifndef _FMOD_H
#define _FMOD_H

#define NSOUNDS 3
extern FMOD_CHANNEL *channel;
extern int fmod_cleanup(void);
extern int fmod_init(void);
extern int fmod_createsound(char *fname, int i);
extern int fmod_playsound(int i);
extern int fmod_setmode(int i);
extern int fmod_getlength(int i, unsigned int *lenms);
extern int fmod_systemupdate(void);
extern int fmod_getchannelsplaying(int *channelsplaying);

#endif //_FMOD_H

