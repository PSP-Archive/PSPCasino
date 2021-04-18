/***********************************************************************************

  Module :	Types.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 24 May 2005 T Swann

***********************************************************************************/

#ifndef _TYPES_H_
#define _TYPES_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************

//**********************************************************************************
//   Macros
//**********************************************************************************
#define MAX( a, b )		( ( a ) > ( b ) ? ( a ) : ( b ) )
#define MIN( a, b )		( ( a ) < ( b ) ? ( a ) : ( b ) )
#define SETMIN( a, b )	{ if ( ( b ) > ( a ) )	( a ) = ( b ); }
#define SETMAX( a, b )	{ if ( ( b ) < ( a ) )	( a ) = ( b ); }
#define SQUARE( x )		( ( x ) * ( x ) )

//**********************************************************************************
//   Types
//**********************************************************************************
typedef signed long				s32;
typedef unsigned long			u32;
typedef signed short			s16;
typedef unsigned short			u16;
typedef char					s8;
typedef unsigned char			u8;
typedef float					f32;
typedef	u32						bool;
typedef unsigned long long		u64;
typedef long long				s64;

//**********************************************************************************
//   Constants
//**********************************************************************************
#define	true	1
#define	false	0

//**********************************************************************************
//   Class definitions
//**********************************************************************************
u32	sceKernelLibcClock();

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* _TYPES_H_ */