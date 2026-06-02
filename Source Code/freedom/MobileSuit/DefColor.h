#ifndef FREEDOM_DRAGONICA_UI_DEFCOLOR_H
#define FREEDOM_DRAGONICA_UI_DEFCOLOR_H
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

/*
DWORD COLOR(a, r, g, b)
{
	return ((a<<24) | (r<<16) | (g<<8) | b);
}
*/
#define COLOR(a, r, g, b) ((a<<24) | (r<<16) | (g<<8) | b)
#define DEF_COLOR( name, r, g, b )	DWORD const COLOR_##name = COLOR(255, r, g, b)

DEF_COLOR(WHITE,	255,255,255);
DEF_COLOR(BLACK,	0,0,0);

DEF_COLOR(RED,		255,0,0);
DEF_COLOR(GREEN,	0,255,0);
DEF_COLOR(BLUE,	0,0,255);

DEF_COLOR(SKY,		0,255,255);
DEF_COLOR(VILOET,	255,0,255);
DEF_COLOR(YELLOW,	255,255,0);

DEF_COLOR(BLOOD,	255,30,15);//ÇÇ»ö
#endif //FREEDOM_DRAGONICA_UI_DEFCOLOR_H