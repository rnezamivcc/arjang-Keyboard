#ifndef _RESIZEINFO_H_
#define _RESIZEINFO_H_

#include "wltypes.h"

#define NSCRNRESMODES	12

// resolution modes  width x height
#define RES240x320		0
#define RES320x240		1
#define RES240x240		2
#define RES240x400		3
#define RES400x240		4
#define RES320x320		5
#define RES480x640		6
#define RES640x480		7
#define RES480x480		8
#define RES480x800		9
#define RES800x480		10
#define RES640x640		11

typedef struct screenResolution 
{
	short width;
	short height;
} ScreenResolution;

extern short g_scrnResMode;
extern short g_handOffset;
extern short g_rightHandOffset[NSCRNRESMODES];
extern short g_rowPosition[6][NSCRNRESMODES];

extern short g_supLocationLeft[NSCRNRESMODES];
extern short g_supLocationRight[NSCRNRESMODES];
extern short g_bmWidth[NSCRNRESMODES];
extern short g_bmHeight[NSCRNRESMODES];
extern short g_keyWidth[NSCRNRESMODES];
extern short g_keyHeight[NSCRNRESMODES];

extern BOOL	g_bHighResScrnMode;

void setHandedOffset(BOOL rightHanded);
BOOL isHighResScrnMode();
void setHighResScrnModeFlag();
void setDisplayResolution();
void getScreenResolution(int *screenWidthp, int *screenHeightp);
int getFontHeight();
int calcExtraScreenWidth();


short getLeftEdgeScrn();
short getRightEdgeScrn();
short getHeightScrn();

#endif
