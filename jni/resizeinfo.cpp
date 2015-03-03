
#include "resizeinfo.h"
#include "utility.h"

// resolution modes  width x height
#ifndef WIN32

short g_fontHeight[NSCRNRESMODES] = 
{
	9,		// portait 240x320
	9,		// landscape 320x240
	9,		// square 240x240
	9,		// portait 240x400
	9,		// landscape 400x240
	7,		// landscape 320x320
	9,	// portait 480x640
	9,	// landscape 640x480
	9,	// square 480x480
	9,	// portrait 480x800
	9,	// landscape 800x480
	9		// square 640x640
};


short g_rightHandOffset[NSCRNRESMODES] = 
{
	77,		// portait 240x320
	77+80,		// landscape 320x240
	77,		// square 240x240
	77,		// portait 240x400
	77+160,		// landscape 400x240
	77+80,		// landscape 320x320   
	154,	// portait 480x640
	154+160,	// landscape 640x480
	154,	// square 480x480
	154,	// portrait 480x800
	154+320,	// landscape 800x480
	154+160		// square 640x640     
};

// row positions for the onscreen keyboard
short g_rowPosition[6][NSCRNRESMODES] = 
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },				//row1 for all different screen res modes
	{ 7, 7, 7, 7, 7, 7, 17, 17, 17, 17, 17, 17 },				// row2 for all diff screen res modes
	{ 15, 15, 15, 15, 15, 15, 31, 31, 31, 31, 31, 31 },         // row3
	{ 47, 47, 47, 47, 47, 47, 95, 95, 95, 95, 95, 95 }, 		// row 4 space bar   old 93
	{ 96, 96, 96, 96, 96, 96, 191, 191, 191, 191, 191, 191 },			// row 5 top row of character keys
 	{ 111, 111, 111, 111, 111, 111, 220, 220, 220, 220, 220, 220 }	// row 6 bottom row of one key 
};

short g_supLocationLeft[NSCRNRESMODES] = 
{
	161,		// portait 240x320
	161,		// landscape 320x240
	161,		// square 240x240
	161,		// portait 240x400
	161,		// landscape 400x240
	161,		// square 320x320
	322,	// portait 480x640
	322,	// landscape 640x480
	322,	// square 480x480
	322,	// portrait 480x800
	322,	// landscape 800x480
	322		// square 640x640
};
short g_supLocationRight[NSCRNRESMODES] = 
{
	2,		// portait 240x320
	2,		// landscape 320x240
	2,		// square 240x240
	2,		// portait 240x400
	2,		// landscape 400x240
	2,		// square 320x320
	4,		// portait 480x640
	4,		// landscape 640x480
	4,		// square 480x480
	4,		// portrait 480x800
	4,		// landscape 800x480
	4		// square 640x640
};

#if 0
short g_supLocationLeft[NSCRNRESMODES] = 
{
	2,		// portait 240x320
	2,		// landscape 320x240
	2,		// square 240x240
	2,		// portait 240x400
	2,		// landscape 400x240
	2,		// square 320x320
	4,	// portait 480x640
	4,	// landscape 640x480
	4,	// square 480x480
	4,	// portrait 480x800
	4,	// landscape 800x480
	4		// square 640x640
};
short g_supLocationRight[NSCRNRESMODES] = 
{
	77 - 40,		// portait 240x320
	77+80 - 40,		// landscape 320x240
	77 - 40,		// square 240x240
	77 - 40,		// portait 240x400
	77+160 - 40,		// landscape 400x240
	77+80 - 40,		// landscape 320x320   
	154 - 40,	// portait 480x640
	154+160 - 40,	// landscape 640x480
	154 - 40,	// square 480x480
	154- 40 ,	// portrait 480x800
	154+320,	// landscape 800x480
	154+160		// square 640x640     
};
#endif


ScreenResolution g_screenResolutions[NSCRNRESMODES] = 
{
	{ 240, 320 },
	{ 320, 240 },
	{ 240, 240 },
	{ 240, 400 },
	{ 400, 240 },
	{ 320, 320 },
	{ 480, 640 },
	{ 640, 480 },
	{ 480, 480 },
	{ 480, 800 },
	{ 800, 480 },
	{ 640, 640 }
};

short g_bmWidth[NSCRNRESMODES] = 
{
	240,		// portait 240x320
	320,		// landscape 320x240
	240,		// square 240x240
	240,		// portait 240x400
	400,		// landscape 400x240
	320,		// square 320x320
	480,		// portait 480x640
	640,		// landscape 640x480
	480,		// square 480x480
	480,		// portrait 480x800
	800,		// landscape 800x480
	640 		// square 640x640
};

short g_bmHeight[NSCRNRESMODES] = 
{
	100,		// portait 240x320
	100,		// landscape 320x240
	100,		// square 240x240
	100,		// portait 240x400
	100,		// landscape 400x240
	100,		// square 320x320
	200,		// portait 480x640
	200,		// landscape 640x480
	200,		// square 480x480
	200,		// portrait 480x800
	200,		// landscape 800x480
	200 		// square 640x640
};

short g_keyWidth[NSCRNRESMODES] = { 16, 16, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32 };
short g_keyHeight[NSCRNRESMODES] = { 16, 16, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32 };

short g_scrnResMode = RES240x320;
short g_handOffset = g_rightHandOffset[RES240x320];
BOOL  g_bHighResScrnMode = FALSE;


void setHandedOffset(BOOL rightHanded)
{
	g_handOffset = rightHanded ? g_rightHandOffset[g_scrnResMode] : 1;
}

short getLeftEdgeScrn()
{
	return 0; // independent of screen modes
}

short getRightEdgeScrn()
{
	return g_bmWidth[g_scrnResMode]; 
}
short getHeightScrn()
{
	return g_bmHeight[g_scrnResMode];
}

void setHighResScrnModeFlag()
{
	if (g_scrnResMode >= RES480x640)
		g_bHighResScrnMode = TRUE;
	else
		g_bHighResScrnMode = FALSE;
}

BOOL isHighResScrnMode()
{
	return g_bHighResScrnMode;
}

void setDisplayResolution()
{
#if !defined(WIN32)
	// Get the width of the screen to determine the layout we start in
	int nWidth = GetSystemMetrics(SM_CXSCREEN);
	int nHeight = GetSystemMetrics(SM_CYSCREEN);

	SPF2(TEXT("setDisplayResolution is %d x %d "), nWidth, nHeight);

	for (int i = 0; i < NSCRNRESMODES; i++)
	{
		if (g_screenResolutions[i].width == nWidth && 
			g_screenResolutions[i].height == nHeight)
		{
			g_scrnResMode = i;
//			g_scrnResMode = 3;
			setHighResScrnModeFlag();
			SPF3(TEXT("setDisplayResolution index is  %d  highscreen %d, fontHeight %d"),
				i, g_bHighResScrnMode, getFontHeight());
			return;
		}
	}
	SPF2(TEXT("UNRESOLVED !!! setDisplayResolution is %d x %d "), nWidth, nHeight);
#endif
}

void getScreenResolution(int *screenWidthp, int *screenHeightp)
{
	*screenHeightp = g_screenResolutions[g_scrnResMode].height;
	*screenWidthp  = g_screenResolutions[g_scrnResMode].width;
}

int calcExtraScreenWidth()
{
	// keyboard is configured to a default 240 or 480 width
	// in case it is wider calc the extra width
	if (g_bHighResScrnMode)
		return g_screenResolutions[g_scrnResMode].width - 480;
	else
		return g_screenResolutions[g_scrnResMode].width - 240;
}

int getFontHeight()
{
	return g_fontHeight[g_scrnResMode];
}
#endif
