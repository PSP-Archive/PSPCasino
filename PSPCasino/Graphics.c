/***********************************************************************************

  Module :	Graphics.c

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 30 May 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "Graphics.h"
#include "font.c"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
#define	CMAX_X	60
#define	CMAX_Y	38
#define	CMAX2_X	30
#define	CMAX2_Y	19
#define	CMAX4_X	15
#define	CMAX4_Y	9

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************
void	sceDisplayWaitVblankStart();
void	sceDisplaySetMode( long, long, long );
void	sceDisplaySetFrameBuf( char * topaddr, long linesize, long pixelsize, long );

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
static char *	pg_vramtop = (char *)0x04000000;
static s32		pg_screenmode;
static s32		pg_showframe;
static s32		pg_drawframe;

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
void pgWaitVn( u32 count )
{
	for (; count>0; --count)
	{
		sceDisplayWaitVblankStart();
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgWaitV()
{
	sceDisplayWaitVblankStart();
}

//*************************************************************************************
//	
//*************************************************************************************
u16 *	pgGetFrame()
{
	return (u16 *)( pg_vramtop + ( pg_drawframe ? FRAMESIZE : 0 ) + 0x40000000 );
}

//*************************************************************************************
//	
//*************************************************************************************
u8 *	pgGetVramAddr( u32 x, u32 y )
{
	return pg_vramtop + ( pg_drawframe ? FRAMESIZE : 0 ) + x * PIXEL_SIZE * 2 + y * LINE_SIZE * 2 + 0x40000000;
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgInit()
{
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	pgScreenFrame( 0, 1 );
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgPrint( u32 x, u32 y, u16 color, const char * str, u8 alpha )
{
	while ( *str != 0 && x < ( CMAX_X * 8 ) && y < ( CMAX_Y * 8 ) )
	{
		pgPutChar( x, y, color, *str, 1, alpha );

		str++;
		x += 8;

		if ( x >= ( CMAX_X * 8 ) )
		{
			return;
		}
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgPrint2( u32 x, u32 y, u16 color, const char * str, u8 alpha )
{
	while ( *str != 0 && x < ( CMAX2_X * 16 ) && y < ( CMAX2_Y * 16 ) )
	{
		pgPutChar( x, y, color, *str, 2, alpha );

		str++;
		x += 16;

		if ( x >= ( CMAX2_X * 16 ) )
		{
			return;
		}
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgPrint4( u32 x, u32 y, u16 color, const char * str, u8 alpha )
{
	while ( *str != 0 && x < ( CMAX4_X * 32 ) && y < ( CMAX4_Y * 32 ) )
	{
		pgPutChar( x, y, color, *str, 4, alpha );

		str++;
		x += 32;

		if ( x >= ( CMAX4_X * 32 ) )
		{
			return;
		}
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgFillvram( u16 color )
{
	u32		i;
	u8 *	vptr0 = pgGetVramAddr(0,0);

	for ( i = 0; i < FRAMESIZE / 2; i++ )
	{
		*(u16 *)vptr0 = color;

		vptr0 += PIXEL_SIZE * 2;
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgBitBlt( u32 x, u32 y, u32 w, u32 h, u32 mag, const u16 * d )
{
	u8 *		vptr;
	u8 *		vptr0 = pgGetVramAddr( x, y );
	u32			xx, yy, mx, my;
	const u16 *	dd;

	for ( yy = 0; yy < h; yy++ )
	{
		for ( my = 0; my < mag; my++ )
		{
			dd = d;
			vptr = vptr0;

			for ( xx = 0; xx < w; xx++ )
			{
				for ( mx = 0; mx < mag; mx++ )
				{
					*(u16 *)vptr = *dd;

					vptr += PIXEL_SIZE * 2;
				}
				dd++;
			}
			vptr0 += LINE_SIZE * 2;
		}
		d += w;
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgPutChar( u32 x, u32 y, u16 color, u8 ch, s8 mag, u8 alpha )
{
	u8 *		vptr0;		//pointer to vram
	u8 *		vptr;		//pointer to vram
	const u8 *	cfont;		//pointer to font
	u32			b;
	u32			cx, cy;
	char		mx, my;

	if ( ch > 255 )
	{
		return;
	}

	cfont = font + ch * 8;

	vptr0 = pgGetVramAddr( x, y );

	for ( cy = 0; cy < 8; cy++ )
	{
		for ( my = 0; my < mag; my++ )
		{
			b = 0x80;
			vptr = vptr0;

			for ( cx = 0; cx < 8; cx++ )
			{
				for ( mx = 0; mx < mag; mx++ )
				{
					if ( ( *cfont & b ) !=0 )
					{
						*(u16 *)( vptr ) = BlendRGB( *(u16 *)( vptr ), color, alpha );
					}

					vptr += PIXEL_SIZE * 2;
				}

				b = b >> 1;
			}

			vptr0 += LINE_SIZE * 2;
		}

		cfont++;
	}
}


//*************************************************************************************
//	
//*************************************************************************************
void	pgScreenFrame( s32 mode, s32 frame )
{
	pg_screenmode = mode;
	frame = ( frame ? 1 : 0 );
	pg_showframe = frame;

	switch ( mode )
	{
		case 0:
		{
			//screen off
			pg_drawframe = frame;
			sceDisplaySetFrameBuf( 0, 0, 0, 1 );
		}
		break;

		case 1:
		{
			//show/draw same
			pg_drawframe = frame;
			sceDisplaySetFrameBuf( pg_vramtop + ( pg_showframe ? FRAMESIZE : 0 ), LINE_SIZE, PIXEL_SIZE, 1 );
		}
		break;

		case 2:
		{
			//show/draw different
			pg_drawframe = ( frame ? 0 : 1 );
			sceDisplaySetFrameBuf( pg_vramtop + ( pg_showframe ? FRAMESIZE : 0 ), LINE_SIZE, PIXEL_SIZE, 1 );
		}
		break;
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgScreenFlip()
{
	pg_showframe = ( pg_showframe ? 0 : 1 );
	pg_drawframe = ( pg_drawframe ? 0 : 1 );

	sceDisplaySetFrameBuf( pg_vramtop + ( pg_showframe ? FRAMESIZE : 0 ), LINE_SIZE, PIXEL_SIZE, 0 );
}

//*************************************************************************************
//	
//*************************************************************************************
void	pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}

//*************************************************************************************
//	
//*************************************************************************************
char *	NumberToAscii( s32 orig_val, char * p_string )
{
	char *	p_work = p_string;

	if ( orig_val < 0 )
	{
		orig_val = -orig_val;

		p_work[ 0 ] = '-';
		p_work++;
	}

	if ( orig_val == 0 )
	{
		p_work[ 0 ] = '0';
		p_work[ 1 ] = '\0';
		p_work++;
	}
	else
	{
		u32	val = orig_val;
		u32	num_digits = 0;

		while ( val != 0 )
		{
			++num_digits;

			val /= 10;
		}

		val = orig_val;
		p_work[ num_digits ] = '\0';

		while ( num_digits > 0 )
		{
			u32	num = val % 10;

			p_work[ num_digits - 1 ] = '0' + num;

			val /= 10;

			--num_digits;
		}
	}

	return p_string;
}

//*************************************************************************************
//	Cheap debug line drawing function
//*************************************************************************************
void	DRAW_LINE( s32 sx, s32 sy, s32 ex, s32 ey, u16 color )
{
	u16 *	p_frame = pgGetFrame();

	if ( sy <= ey )
	{
		s32		dx = ex - sx + 1;
		s32		dy = ey - sy + 1;
		s32		slope = ( dy == 0 ? 0 : ( dx << 16 ) / dy );
		s32		x = sx << 16;
		s32		y;

		for ( y = sy; y < ey; ++y )
		{
			p_frame[ ( x >> 16 ) + ( y * LINE_SIZE ) ] = color;

			x = x + slope;
		}
	}
	else
	{
		s32		dx = sx - ex + 1;
		s32		dy = ey - sy + 1;
		s32		slope = ( dy == 0 ? 0 : ( dx << 16 ) / dy );
		s32		x = sx << 16;
		s32		y;

		for ( y = sy; y > ey; --y )
		{
			p_frame[ ( x >> 16 ) + ( y * LINE_SIZE ) ] = color;

			x = x + slope;
		}
	}
}

//*************************************************************************************
//	
//*************************************************************************************
inline u16	BlendRGB( u16 rgb1, u16 rgb2, u8 t )
{
	const s32	r1 = R( rgb1 );
	const s32	g1 = G( rgb1 );
	const s32	b1 = B( rgb1 );
	const s32	r2 = R( rgb2 );
	const s32	g2 = G( rgb2 );
	const s32	b2 = B( rgb2 );

	const s32	rd = ( r2 - r1 ) * t;
	const s32	gd = ( g2 - g1 ) * t;
	const s32	bd = ( b2 - b1 ) * t;

	const s32	r = r1 + ( rd >> 8 );
	const s32	g = g1 + ( gd >> 8 );
	const s32	b = b1 + ( bd >> 8 );

	return RGB5( r, g, b );
}

//*******************************  END OF FILE  ************************************
