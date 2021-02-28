#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
//#define GUI_SUPPORT_CURSOR (1)
#include "GUI.h"
#include "LCDConf.h" // Required for config check
#include "mtb_st7789v.h"
#include "cy8ckit_028_tft_pins.h" /* This is part of the CY8CKIT-028-TFT shield library. */
#include "FreeRTOS.h"
#include "task.h"
#include "images.h"

#include "segemwin.h"

#define RECOMMENDED_MEMORY (1024L * 10)  // for cursor and sprite demos



/* The pins above are defined by the CY8CKIT-028-TFT library. If the display is being used on different hardware the mappings will be different. */
const mtb_st7789v_pins_t tft_pins =
{
    .db08 = CY8CKIT_028_TFT_PIN_DISPLAY_DB8,
    .db09 = CY8CKIT_028_TFT_PIN_DISPLAY_DB9,
    .db10 = CY8CKIT_028_TFT_PIN_DISPLAY_DB10,
    .db11 = CY8CKIT_028_TFT_PIN_DISPLAY_DB11,
    .db12 = CY8CKIT_028_TFT_PIN_DISPLAY_DB12,
    .db13 = CY8CKIT_028_TFT_PIN_DISPLAY_DB13,
    .db14 = CY8CKIT_028_TFT_PIN_DISPLAY_DB14,
    .db15 = CY8CKIT_028_TFT_PIN_DISPLAY_DB15,
    .nrd  = CY8CKIT_028_TFT_PIN_DISPLAY_NRD,
    .nwr  = CY8CKIT_028_TFT_PIN_DISPLAY_NWR,
    .dc   = CY8CKIT_028_TFT_PIN_DISPLAY_DC,
    .rst  = CY8CKIT_028_TFT_PIN_DISPLAY_RST
};
/* Number of demo pages */
#define NUMBER_OF_DEMO_PAGES    (12u)

/* Delay after startup screen in milliseconds */
#define DELAY_AFTER_STARTUP_SCREEN_MS       (2000)

/* External global references */
//extern GUI_CONST_STORAGE GUI_BITMAP bmCypressLogo_1bpp;

extern GUI_CONST_STORAGE GUI_BITMAP bmExampleImage;
extern GUI_CONST_STORAGE GUI_BITMAP bmCypressLogo;

/* Function prototypes */
void ShowTextModes(void);
void ShowTextColors(void);
void ShowFontSizesNormal(void);
void ShowFontSizesBold(void);
void ShowColorBar(void);
void Show2DGraphics1(void);
void Show2DGraphics2(void);
void ShowConcentricCircles(void);
void ShowTextWrapAndOrientation(void);
void ShowBitmap(void);
void ShowCursor(void);
void ShowSprites(void);

/* Array of demo pages functions */
void (*demoPageArray[NUMBER_OF_DEMO_PAGES])(void) = {
    ShowTextModes,
    ShowTextColors,
    ShowFontSizesNormal,
    ShowFontSizesBold,
    ShowTextWrapAndOrientation,
    ShowColorBar,
    Show2DGraphics1,
    Show2DGraphics2,
    ShowConcentricCircles,
    ShowBitmap,
	ShowCursor,
	ShowSprites
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _MoveSprite
*/
static void _MoveSprite(SPRITE * pSprite) {
  //
  // Set new index
  //
  pSprite->Index += 1;
  if (pSprite->Index >= pSprite->NumImages) {
    pSprite->Index = 0;
  }
  //
  // Adjust X-position
  //
  pSprite->xPos += pSprite->dx;
  if (pSprite->xPos > 320) {
    pSprite->xPos = -1L * pSprite->apImages[pSprite->Index]->XSize;
  }
  if (pSprite->xPos < -1L * pSprite->apImages[pSprite->Index]->XSize) {
    pSprite->xPos = 320;
  }
  //
  // Adjust Y-position
  //
  pSprite->yPos += pSprite->dy;
  if (pSprite->yPos < 0) {
    pSprite->dy = -pSprite->dy;
  }
  if (pSprite->yPos > (240 - pSprite->apImages[pSprite->Index]->YSize)) {
    pSprite->dy = -1 * pSprite->dy;
  }
  //
  // Change sprite
  //
  GUI_SPRITE_SetBitmapAndPosition(pSprite->hSprite, pSprite->apImages[pSprite->Index], pSprite->xPos, pSprite->yPos);
}

/*********************************************************************
*
*       _ShowSprites
*/
static void _ShowSprites(void) {
  unsigned i;

  //
  // Create the sprites
  //
  for (i = 0; i < GUI_COUNTOF(_aSprite); i++) {
    _aSprite[i].hSprite = GUI_SPRITE_Create(_aSprite[i].apImages[0], _aSprite[i].xPos, _aSprite[i].yPos);
  }
  //
  // Move them forever...
  //
  while (1) {
    for (i = 0; i < GUI_COUNTOF(_aSprite); i++) {
      _MoveSprite(&_aSprite[i]);
      GUI_Delay(100);
    }
  }
}

/*********************************************************************
*
*       _DrawBackground
*
**********************************************************************/
static void _DrawBackground(void) {
  GUI_DrawGradientV(0, 0, 319, 239, GUI_MAKE_COLOR(0xffffff), GUI_MAKE_COLOR(0xff0000));  // Draw background gradient
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetFont(&GUI_Font24B_ASCII);
  GUI_SetColor(GUI_BLACK);
  GUI_DispStringHCenterAt("Showing sprites...", 160, 20); // Draw sample text
//  GUI_DrawBitmap(&bmSeggerLogoBlue, 60, 80);             // Draw company logo
}

/*********************************************************************
*
*       _MoveCursor
*/
static void _MoveCursor(void) {
  int x;
  int y;
  int tm;
  int cnt;
  int yStep;
  int xPos;
  int yPos;

  cnt  = 0;
  yStep=-1;
  xPos = LCD_GetXSize() / 2 - bmSeggerLogoBlue.XSize/2;
  yPos = LCD_GetYSize() / 2 - bmSeggerLogoBlue.YSize/2+25;
  GUI_DispStringHCenterAt("Cursor shape can be changed\nand the cursor can be moved", 160,  75);
  GUI_CURSOR_Show();
  GUI_DrawBitmap(&bmSeggerLogoBlue, xPos, yPos );
  y = 150;
  for (x = 0; x < 320; x++) {
    if ((x % 54) == 0) {
      GUI_CURSOR_Select(_apCursor[cnt++]);
    }
    tm = GUI_GetTime();
    y += yStep;
    if(y<=80) yStep=1;
    if(y>=150) yStep=-1;
    GUI_CURSOR_SetPosition(x, y);
    while ((GUI_GetTime() - tm) < 10);
  }
  for (x = 320; x > 0; x--) {
    tm = GUI_GetTime();
    if ((x % 54) == 0) {
      GUI_CURSOR_Select(_apCursor[cnt++]);
    }
    y += yStep;
    if(y<=80) yStep=1;
    if(y>=150) yStep=-1;
    GUI_CURSOR_SetPosition(x, y);
    while ((GUI_GetTime() - tm) < 10);
  }
  GUI_CURSOR_Hide();
  GUI_Delay(500);
}

/*********************************************************************
*
*       _DispCursor
*/
static void _DispCursor(void) {
  int i;
  int x;
  int y;

  GUI_DispStringHCenterAt("Available cursors:", 160,  80);
  for (i = 0; i < 12; i++) {
    x = 160 - (_apCursor[i]->pBitmap->XSize / 2);
    y = 120 - (_apCursor[i]->pBitmap->YSize / 2);
    GUI_DrawBitmap(_apCursor[i]->pBitmap, x, y);
    GUI_DispStringHCenterAt(_aacCursorName[i], 160,145);
    GUI_Delay(750);
    GUI_ClearRect(0, 100, 319, 165);
  }
  GUI_ClearRect(0, 80, 319, 100);
  GUI_Delay(500);
}

/*********************************************************************
*
*       _DemoCursor
*/
static void _DemoCursor(void) {
  GUI_SetBkColor(GUI_BLUE);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("CURSOR_Sample - 11 OF 12", 160, 5);
  GUI_SetFont(&GUI_Font8x16);
//  while (1) {
    _DispCursor();
    GUI_ClearRect(0, 60, 319, 200);
    _MoveCursor();
    GUI_ClearRect(0, 60, 319, 200);
//  }
}

/*******************************************************************************
* Function Name: void ShowStartupScreen(void)
********************************************************************************
*
* Summary: This function displays the startup screen.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowStartupScreen(void)
{
    /* Set font size, foreground and background Colours */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);

    /* Clear the screen */
    GUI_Clear();

    /* Draw the Cypress Logo */
    GUI_DrawBitmap(&bmCypressLogo, 4, 4);

    /* Print the text CYPRESS EMWIN GRAPHICS DEMO TFT DISPLAY */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CYPRESS", 160, 120);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EMWIN GRAPHICS DEMO", 160, 140);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TFT DISPLAY", 160, 160);
}


/*******************************************************************************
* Function Name: void ShowInstructionsScreen(void)
********************************************************************************
*
* Summary: This function shows screen with instructions to press SW2 to
*            scroll through various display pages
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowInstructionsScreen(void)
{
    /* Set font size, background Colour and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display instructions text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("PRESS SW2 ON THE KIT", 160, 90);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TO SCROLL THROUGH ", 160, 110);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("DEMO PAGES!", 160, 130);
}


/*******************************************************************************
* Function Name: void ShowTextModes(void)
********************************************************************************
*
* Summary: This function displays the following
*            1. Left, Center and Right aligned text
*            2. Underline, overline and strikethrough style text
*            3. Normal, reverse, transparent and XOR text modes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowTextModes(void)
{
    /* Set font size, foreground and background Colours */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("1 OF 12: TEXT ALIGNMENTS, STYLES AND MODES", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Display left aligned text */
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT ALIGNMENT LEFT", 0, 40);

    /* Display center aligned text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TEXT ALIGNMENT CENTER", 160, 60);

    /* Display right aligned text */
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_DispStringAt("TEXT ALIGNMENT RIGHT", 319, 80);

    /* Display underlined text */
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE UNDERLINE", 0, 100);

    /* Display overlined text */
    GUI_SetTextStyle(GUI_TS_OVERLINE);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE OVERLINE", 0, 120);

    /* Display strikethrough text */
    GUI_SetTextStyle(GUI_TS_STRIKETHRU);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE STRIKETHROUGH", 0, 140);

    /* Create a rectangle filled with blue Colour */
    GUI_SetColor(GUI_BROWN);
    GUI_FillRect(0, 160, 319, 239);

    /* Draw two diagonal lines */
    GUI_SetColor(GUI_BLUE);
    GUI_SetPenSize(3);
    GUI_DrawLine(0, 160, 319, 239);
    GUI_DrawLine(0, 239, 319, 160);

    /* Set text Colour to white with black background */
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);

    /* Set text style to normal */
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Display text in normal mode. This will print white text in
        a black box */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_DispStringAt("TEXT MODE NORMAL", 160, 165);

    /* Display text in reverse mode. This will print black text n
        a white box */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_REV);
    GUI_DispStringAt("TEXT MODE REVERSE", 160, 185);

    /* Display transparent text.  This will display white text
        on the blue background already present in the rectangle */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_DispStringAt("TEXT MODE TRANSPARENT", 160, 205);

    /* Display XOR text.  This will XOR the blue background with
        the white text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_XOR);
    GUI_DispStringAt("TEXT MODE XOR", 160, 225);
}


/*******************************************************************************
* Function Name: void ShowTextColors(void)
********************************************************************************
*
* Summary: This function displays text in various Colours
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowTextColors(void)
{
    /* Set font size, background Colour and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("2 OF 12: TEXT COLOURS", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* White */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("TEXT COLOUR WHITE", 160, 40);

    /* Gray */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_GRAY);
    GUI_DispStringAt("TEXT COLOUR GRAY", 160, 60);

    /* Red */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_RED);
    GUI_DispStringAt("TEXT COLOUR RED", 160, 80);

    /* Green */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_GREEN);
    GUI_DispStringAt("TEXT COLOUR GREEN", 160, 100);

    /* Blue */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_BLUE);
    GUI_DispStringAt("TEXT COLOUR BLUE", 160, 120);

    /* Yellow */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_YELLOW);
    GUI_DispStringAt("TEXT COLOUR YELLOW", 160, 140);

    /* Brown */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_BROWN);
    GUI_DispStringAt("TEXT COLOUR BROWN", 160, 160);

    /* Magenta */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_MAGENTA);
    GUI_DispStringAt("TEXT COLOUR MAGENTA", 160, 180);

    /* Cyan */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_CYAN);
    GUI_DispStringAt("TEXT COLOUR CYAN", 160, 200);

    /* Orange */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetColor(GUI_ORANGE);
    GUI_DispStringAt("TEXT COLOUR ORANGE", 160, 220);
}


/*******************************************************************************
* Function Name: void ShowFontSizesNormal(void)
********************************************************************************
*
* Summary: This function shows various font sizes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowFontSizesNormal(void)
{
    /* Set font size, background colour and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetColor(GUI_GRAY);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("3 OF 12: NORMAL FONTS", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Font8_1*/
    GUI_SetFont(GUI_FONT_8_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_GRAY);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_DispStringAt("GUI_Font8_1", 10, 40);

    /* Font10_1*/
    GUI_SetFont(GUI_FONT_10_1);
    GUI_DispStringAt("GUI_Font10_1", 10, 50);

    /* Font13_1*/
    GUI_SetFont(GUI_FONT_13_1);
    GUI_DispStringAt("GUI_Font13_1", 10, 62);

    /* Font16_1*/
    GUI_SetFont(GUI_FONT_16_1);
    GUI_DispStringAt("GUI_Font16_1", 10, 77);

    /* Font20_1*/
    GUI_SetFont(GUI_FONT_20_1);
    GUI_DispStringAt("GUI_Font20_1", 10, 95);

    /* Font24_1*/
    GUI_SetFont(GUI_FONT_24_1);
    GUI_DispStringAt("GUI_Font24_1", 10, 117);

    /* Font32_1*/
    GUI_SetFont(GUI_FONT_32_1);
    GUI_DispStringAt("GUI_Font32_1", 10, 143);

    /* Font48_1*/
    GUI_SetFont(GUI_FONT_8X16X2X2);
    GUI_DispStringAt("GUI_Font8x16x2x2", 10, 180);
}


/*******************************************************************************
* Function Name: void ShowFontSizesBold(void)
********************************************************************************
*
* Summary: This function shows various font sizes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowFontSizesBold(void)
{
    /* Set font size, background colour and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_GRAY);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("4 OF 12: BOLD FONTS", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Font13B_1*/
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_DispStringAt("GUI_Font13B_1", 10, 40);

    /* Font13HB_1*/
    GUI_SetFont(GUI_FONT_13HB_1);
    GUI_DispStringAt("GUI_Font13HB_1", 10, 55);

    /* Font16B_1*/
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_DispStringAt("GUI_Font6B_1", 10, 70);

    /* FontComic18B_1*/
    GUI_SetFont(GUI_FONT_COMIC18B_1);
    GUI_DispStringAt("GUI_FontComic18B_1", 10, 88);

    /* Font20B_1*/
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_DispStringAt("GUI_Font20B_1", 10, 108);

    /* Font24B_1*/
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_DispStringAt("GUI_Font24B_1", 10, 130);

    /* Font32B_1*/
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_DispStringAt("GUI_Font32B_1", 10, 156);

    /* Font48B_1*/
    GUI_SetFont(GUI_FONT_8X16X3X3);
    GUI_DispStringAt("GUI_Font8x16x3x3", 10, 190);
}


/*******************************************************************************
* Function Name: void ShowColorBar(void)
********************************************************************************
*
* Summary: This function displays displays horizontal colour bars.  For each
*            colour, two bars are printed, one bar that has a gradient from
*            black to the colour and another bar that has a gradient from white
*            to the colour.
*
* Note: This function is provided by EmWin as a demo code.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowColorBar(void)
{
    /* Local variables */
    int x0;
    int y0;
    int yStep;
    int i;
    int xsize;
    U16 cs;
    U16 x;

    /* Initialize parameters */
    x0        = 60;
    y0        = 40;
    yStep     = 15;
    xsize     = LCD_GetDevCap(LCD_DEVCAP_XSIZE) - x0;

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("6 OF 12: COLOUR BARS", 160, 5);

    /* Set text mode to normal and left alignment */
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Display labels for the bars */
    GUI_SetFont(&GUI_Font8x16);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_DispStringAt("Red",     0, y0 +      yStep);
    GUI_DispStringAt("Green",   0, y0 +  3 * yStep);
    GUI_DispStringAt("Blue",    0, y0 +  5 * yStep);
    GUI_DispStringAt("Yellow",  0, y0 +  7 * yStep);
    GUI_DispStringAt("Cyan",    0, y0 + 9 * yStep);
    GUI_DispStringAt("Magenta", 0, y0 + 11 * yStep);

    /* Draw the colour bars */
    for (i = 0; i < xsize; i++)
    {
        cs = (0xFF * (U32)i) / xsize;
        x = x0 + i;;

        /* Red */
        GUI_SetColor(cs);
        GUI_DrawVLine(x, y0             , y0 +     yStep - 1);
        GUI_SetColor(0xff + (255 - cs) * 0x10100uL);
        GUI_DrawVLine(x, y0 +      yStep, y0 + 2 * yStep - 1);

        /* Green */
        GUI_SetColor(cs<<8);
        GUI_DrawVLine(x, y0 +  2 * yStep, y0 + 3 * yStep - 1);
        GUI_SetColor(0xff00 + (255 - cs) * 0x10001uL);
        GUI_DrawVLine(x, y0 +  3 * yStep, y0 + 4 * yStep - 1);

        /* Blue */
        GUI_SetColor(cs * 0x10000uL);
        GUI_DrawVLine(x, y0 +  4 * yStep, y0 + 5 * yStep - 1);
        GUI_SetColor(0xff0000 + (255 - cs) * 0x101uL);
        GUI_DrawVLine(x, y0 +  5 * yStep, y0 + 6 * yStep - 1);

        /* Yellow */
        GUI_SetColor(cs * 0x101uL);
        GUI_DrawVLine(x, y0 +  6 * yStep, y0 + 7 * yStep - 1);
        GUI_SetColor(0xffff + (255 - cs) * 0x10000uL);
        GUI_DrawVLine(x, y0 +  7 * yStep, y0 + 8 * yStep - 1);

        /* Cyan */
        GUI_SetColor(cs * 0x10100uL);
        GUI_DrawVLine(x, y0 +  8 * yStep, y0 + 9 * yStep - 1);
        GUI_SetColor(0xffff00 + (255 - cs) * 0x1L);
        GUI_DrawVLine(x, y0 + 9 * yStep, y0 + 10 * yStep - 1);

        /* Magenta */
        GUI_SetColor(cs * 0x10001uL);
        GUI_DrawVLine(x, y0 + 10 * yStep, y0 + 11 * yStep - 1);
        GUI_SetColor(0xff00ff + (255 - cs) * 0x100uL);
        GUI_DrawVLine(x, y0 + 11 * yStep, y0 + 12 * yStep - 1);
    }
}


/*******************************************************************************
* Function Name: void Show2DGraphics1(void)
********************************************************************************
*
* Summary: This function displays the following 2D graphics
*            1. Horizontal lines with various pen widths
*            2. Vertical lines with various pen widths
*            3. Arcs
*            4. Filled rounded rectangle
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Show2DGraphics1(void)
{
    /* Set font size, foreground and background colours */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("7 OF 12: 2D GRAPHICS - 1", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Set drawing colour to gray */
    GUI_SetColor(GUI_GRAY);

    /* Display labels */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("H-LINES", 80, 110);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("V-LINES", 240, 110);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ARCS", 80, 220);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ROUNDED RECT", 240, 220);

    /* Horizontal lines */
    GUI_SetPenSize(1);
    GUI_DrawLine(10, 35, 150, 35);
    GUI_SetPenSize(2);
    GUI_DrawLine(10, 50, 150, 50);
    GUI_SetPenSize(3);
    GUI_DrawLine(10, 65, 150, 65);
    GUI_SetPenSize(4);
    GUI_DrawLine(10, 80, 150, 80);
    GUI_SetPenSize(5);
    GUI_DrawLine(10, 95, 150, 95);

    /* Vertical lines */
    GUI_SetPenSize(1);
    GUI_DrawLine(210, 30, 210, 100);
    GUI_SetPenSize(2);
    GUI_DrawLine(225, 30, 225, 100);
    GUI_SetPenSize(3);
    GUI_DrawLine(240, 30, 240, 100);
    GUI_SetPenSize(4);
    GUI_DrawLine(255, 30, 255, 100);
    GUI_SetPenSize(5);
    GUI_DrawLine(270, 30, 270, 100);

    /* Arcs */
    GUI_SetPenSize(2);
    GUI_DrawArc(80, 210, 10, 10, 0, 180);
    GUI_DrawArc(80, 210, 20, 20, 0, 180);
    GUI_DrawArc(80, 210, 30, 30, 0, 180);
    GUI_DrawArc(80, 210, 40, 40, 0, 180);
    GUI_DrawArc(80, 210, 50, 50, 0, 180);
    GUI_DrawArc(80, 210, 60, 60, 0, 180);

    /* Rounded rectangle */
    GUI_FillRoundedRect(180, 150, 300, 210, 5);
}


/*******************************************************************************
* Function Name: void Show2DGraphics2(void)
********************************************************************************
*
* Summary: This function displays the following 2D graphics
*            1. Concentric circles
*            2. Concentric ellipses
*            3. Filled rectangle with horizontal colour gradient
*            4. Filled rectangle with vertical colour gradient
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Show2DGraphics2(void)
{
    /* Set font size, foreground and background colours */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_DispStringAt("8 OF 12: 2D GRAPHICS - 2", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Set drawing colour to gray */
    GUI_SetColor(GUI_GRAY);

    /* Display labels */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CIRCLE", 80, 110);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ELLIPSE", 240, 110);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("RECT-HGRAD", 80, 220);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("RECT-VGRAD", 240, 220);

    /* Concentric Circles */
    GUI_DrawCircle(80, 65, 35);
    GUI_DrawCircle(80, 65, 25);
    GUI_DrawCircle(80, 65, 15);
    GUI_DrawCircle(80, 65, 5);

    /* Concentric Ellipses */
    GUI_DrawEllipse(240, 65, 60, 35);
    GUI_DrawEllipse(240, 65, 50, 25);
    GUI_DrawEllipse(240, 65, 40, 15);
    GUI_DrawEllipse(240, 65, 30, 5);

    /* Rectangle horizontal gradient */
    GUI_DrawGradientH(20, 150, 140, 210, GUI_BLACK, GUI_GREEN);

    /* Rectangle vertical gradient */
    GUI_DrawGradientV(180, 150, 300, 210, GUI_RED, GUI_BLACK);
}


/*******************************************************************************
* Function Name: void ShowConcentricCircles(void)
********************************************************************************
*
* Summary: This function displays a short animation by drawing concentric circles
*  with colour gradients
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowConcentricCircles(void)
{
    #define NUMBER_OF_COLOURS    (3u)
    #define MAX_RADIUS          (100u)

    uint16 radius = 0;
    uint16 colourIndex = 0;

    const uint32 colourArray[NUMBER_OF_COLOURS] =
    {
        GUI_RED,
        GUI_BLUE,
        GUI_GREEN,
    };

    /* Set font size, background colour and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("9 OF 12: CONCENTRIC CIRCLES", 160, 10);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Draw concentric circles with colour gradient */
    /* Cycle through number of colours */
    for(colourIndex = 0; colourIndex < NUMBER_OF_COLOURS; colourIndex++)
    {
        /* Draw circles with increasing radius */
        for(radius = 0; radius < MAX_RADIUS; radius++)
        {
            GUI_SetColor(radius*colourArray[colourIndex]);
            GUI_DrawCircle(160, 130, radius);
            CyDelay(2);
        }
    }
}


/*******************************************************************************
* Function Name: void ShowBitmap(void)
********************************************************************************
*
* Summary: This function displays a bitmap image with an overlay text
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowTextWrapAndOrientation(void)
{
    GUI_RECT leftRect = {4, 19, 24, 166};
    GUI_RECT rightRect = {238, 19, 258, 166};
    GUI_RECT middleRect = {29, 19, 233, 166};
    GUI_RECT middleRectMargins = {31, 20, 232, 165};
    GUI_RECT bottomRect = {31, 170, 232, 220};

    const char leftText[] = "ROTATED TEXT CCW";
    const char rightText[] = "ROTATED TEXT CW";
    const char bottomText[] = "INVERTED TEXT 180";

    const char middleText[] = "This project demonstrates displaying 2D graphics in a TFT display using Segger EmWin Graphics Library. \n\nThis page shows the text wrap and text rotation features. In the left rectangle, the text is rotated counter clockwise and in the right rectangle, the text is rotated clockwise.";


    /* Set font size, foreground and background colours */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("5 OF 12: TEXT WRAP AND ROTATION", 132, 5);

    /* Draw rectangles to hold text */
    GUI_DrawRectEx(&leftRect);
    GUI_DrawRectEx(&rightRect);
    GUI_DrawRectEx(&middleRect);
    GUI_DrawRectEx(&bottomRect);

    /* Display string in left rectangle rotated counter clockwise */
    GUI_DispStringInRectEx(leftText, &leftRect, GUI_TA_HCENTER | GUI_TA_VCENTER, strlen(leftText), GUI_ROTATE_CCW);

    /* Display string in right rectangle rotated clockwise */
    GUI_DispStringInRectEx(rightText, &rightRect, GUI_TA_HCENTER | GUI_TA_VCENTER, strlen(rightText), GUI_ROTATE_CW);

    /* Display string in right rectangle rotated clockwise */
    GUI_DispStringInRectEx(bottomText, &bottomRect, GUI_TA_HCENTER | GUI_TA_VCENTER, strlen(bottomText), GUI_ROTATE_180);

    /* Display string in middle rectangle with word wrap */
    GUI_DispStringInRectWrap(middleText, &middleRectMargins, GUI_TA_LEFT, GUI_WRAPMODE_WORD);

}
void ShowBitmap(void)
{
    /* Set background colour to black and clear screen */
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();

    /* Display the bitmap image on the screen */
    GUI_DrawBitmap(&bmExampleImage, 0, 4);

    /* Set font size, font colour to black */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_BLACK);

    /* Set text mode to transparent, underlined and center aligned */
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextStyle(GUI_TS_UNDERLINE);

    /* Print the page title text */
    GUI_DispStringAt("10 OF 12: BITMAP IMAGE", 160, 10);
}
/*******************************************************************************
* Function Name: void ShowCursor(void)
********************************************************************************
*
* Summary: This implements a simple Cursor and moves it about on the displaay
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowCursor(void) {
	  //
	  // Check if recommended memory for the sample is available
	  //
//	  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
//	    GUI_ErrorOut("Not enough memory available.");
//	    return;
//	  }
	  _DemoCursor();

}

void ShowSprites(void) {
	//
	// Check if recommended memory for the sample is available
	//
//	if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
//		GUI_ErrorOut("Not enough memory available.");
//		return;
//	}
	_DrawBackground(); // Draw background
	_ShowSprites();    // Show sprites
}
/*******************************************************************************
* Function Name: void WaitforSwitchPressAndRelease(void)
********************************************************************************
*
* Summary: This implements a simple "Wait for button press and release"
*           function.  It first waits for the button to be pressed and then
*           waits for the button to be released.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This is a blocking function and exits only on a button press and release
*
*******************************************************************************/
void WaitforSwitchPressAndRelease(void)
{
    /* Wait for SW2 to be pressed */
    while( CYBSP_BTN_PRESSED != cyhal_gpio_read(CYBSP_USER_BTN));

    /* Wait for SW2 to be released */
    while( CYBSP_BTN_PRESSED == cyhal_gpio_read(CYBSP_USER_BTN));
}

/*******************************************************************************
* Function Name: int oledTask(void *arg)
********************************************************************************
*
* Summary: This is the entry function of the task.  Following operations are
*           performed in this function
*           1. emWin GUI is initialized
*           2. Startup screen with CY logo is displayed for 2 seconds
*           3. Instructions screen is displayed
*           4. On every key press of SW2 on the kit, the program scrolls through
*               following demo pages
*                   a. Text orientation and text wrap
*                   b. Normal fonts
*                   c. Bold fonts
*                   e. Text alignments
*                   f. Text styles
*                   g. 2D graphics with lines of various thickness, rectangles
*                       and circles.
*
* Parameters:
*  void* arg : argument to task if any
*
* Return:
*  None
*
*******************************************************************************/
void tftTask(void)
{

    cy_rslt_t result;
    uint8_t pageNumber = 0;

    /* Initialize the display controller */
    result = mtb_st7789v_init8(&tft_pins);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* To avoid compiler warning */
    (void)result;

    /* Initialize emWin GUI */
    GUI_Init();

    GUI_DispString("Hello world!");

    /* Configure Switch and LEDs*/
    cyhal_gpio_init( CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                     CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Display startup screen for 2 seconds */
    ShowStartupScreen();
    CyDelay(DELAY_AFTER_STARTUP_SCREEN_MS);
//    vTaskDelay(DELAY_AFTER_STARTUP_SCREEN_MS);

    /* Show the instructions screen */
    ShowInstructionsScreen();

    for (;;)
    {
        /* Wait for a switch press event */
        WaitforSwitchPressAndRelease();

        /* Using pageNumber as index, update the display with a demo screen */
        (*demoPageArray[pageNumber])();

        /* Cycle through page numbers */
        pageNumber = (pageNumber + 1) % NUMBER_OF_DEMO_PAGES;
    }
}
