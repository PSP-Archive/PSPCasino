/***********************************************************************************

  Module :	Graphics.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 28 May 2005 T Swann

***********************************************************************************/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "Types.h"

//**********************************************************************************
//   Macros
//**********************************************************************************
#define RGB5( r, g, b )	( ( ( b ) << 10 ) + ( ( g ) << 5 ) + ( ( r ) << 0 ) + 0x8000 )
#define RGB( r, g, b )	( ( ( ( b ) >> 3 ) << 10 ) + ( ( ( g ) >> 3 ) << 5 ) + ( ( ( r ) >> 3 ) << 0 ) + 0x8000 )
#define R( c )			( ( ( c ) >> 0 ) & 31 )
#define G( c )			( ( ( c ) >> 5 ) & 31 )
#define B( c )			( ( ( c ) >> 10 ) & 31 )

#define	BLEND_RGB( rgb1, rgb2, t )								\
																\
RGB5(	R( rgb1 ) + ( ( R( rgb2 ) - R( rgb1 ) * t ) >> 8 ),		\
		G( rgb1 ) + ( ( G( rgb2 ) - G( rgb1 ) * t ) >> 8 ),		\
		B( rgb1 ) + ( ( B( rgb2 ) - B( rgb1 ) * t ) >> 8 ) )

//**********************************************************************************
//   Types
//**********************************************************************************
typedef struct
{
	s32 r, g, b;
} RGBA;

//**********************************************************************************
//   Constants
//**********************************************************************************
#define	PIXEL_SIZE			1				//in short
#define	LINE_SIZE			512				//in short
#define	FRAMESIZE			0x44000			//in byte

#define	SCREEN_WIDTH		480
#define	SCREEN_HEIGHT		272

//**********************************************************************************
//   Class definitions
//**********************************************************************************
void	pgInit();
void	pgWaitV();
void	pgWaitVn( u32 count );
u16 *	pgGetFrame();
u8 *	pgGetVramAddr( u32 x, u32 y );
void	pgScreenFrame( s32 mode, s32 frame );
void	pgScreenFlip();
void	pgScreenFlipV();
void	pgPrint( u32 x, u32 y, u16 color, const char * str, u8 alpha );
void	pgPrint2( u32 x, u32 y, u16 color, const char * str, u8 alpha );
void	pgPrint4( u32 x, u32 y, u16 color, const char * str, u8 alpha );
void	pgFillvram( u16 color );
void	pgBitBlt( u32 x, u32 y, u32 w, u32 h, u32 mag, const u16 * d );
void	pgPutChar( u32 x, u32 y, u16 color, u8 ch, s8 mag, u8 alpha );

char *	NumberToAscii( s32 orig_val, char * p_string );

void	DRAW_LINE( s32 sx, s32 sy, s32 ex, s32 ey, u16 color );

inline u16	BlendRGB( u16 rgb1, u16 rgb2, u8 t );


//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* _GRAPHICS_H_ */
