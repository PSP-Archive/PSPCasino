/***********************************************************************************

  BlackJack v0.2b for PSP
  Copyright 2005 GlobWare.com
  6/19/2005 v0.1
  6/20/2005 v0.2
  6/20/2005 v0.2b

  areas copyright their respective owners

  solid.c

***********************************************************************************/

/***********************************************************************************

  Module :	SOLID.c

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 28 May 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************

#include "Types.h"
#include "Graphics.h"
#include <string.h>
#include "Controller.h"
#include "bitmap.c"

//-= Function Definitions =- 

#define O_RDONLY    0x0001 
#define O_WRONLY    0x0002 
#define O_RDWR      0x0003 
#define O_NBLOCK    0x0010 
#define O_APPEND    0x0100 
#define O_CREAT     0x0200 
#define O_TRUNC     0x0400 
#define O_NOWAIT    0x8000 

#define DEBUG_X		400
#define DEBUG_Y		256

#define WINDOW_XPOS	10

// File Manager
int sceIoOpen(const char* file, int mode); 
void sceIoClose(int fd); 
int sceIoRead(int fd, void *data, int size); 
int sceIoWrite(int fd, void *data, int size); 
int sceIoLseek(int fd, int offset, int whence); 
int sceIoRemove(const char *file); 
int sceIoMkdir(const char *dir, int mode); 
int sceIoRmdir(const char *dir); 

// My Routines
void	BuildDeck(void);
void	MasterShuffle(void); 
void	DrawCard(int player);
void	PlayDeckShuffle(void);
void	DisplayPlayerHand();
void	CacheKey();

int		CalcPlayer(void);
int		CalcDealer(void);

void	DisplayDealerHand();
void	ProcessGameCmd();
void	DrawDebugInfo();
void	ProcessInput();
void	DisplayGameInstructions();
void	DoDealerTurn();
void	DetermineWinner();
void	DisplayCard(int cardnum,int player,int x,int y);

//**********************************************************************************
//   Local Constants
//**********************************************************************************

enum {
	GS_INIT,
	GS_WELCOME,
	GS_MENU,
	GS_BET,
	GS_STOREBET,
	GS_DEAL,
	GS_PLAYERTURN,
	GS_SHOWDEALER,
	GS_DEALERTURN,
	GS_RESULTS,
	GS_PAYOFF,
	GS_ENDGAME,
	GS_EXITGAME,
	GS_REFRESH,
	GS_GAMESAVED,
	GS_IDLE

} GAMESTATES;

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************
void	__exit();

//**********************************************************************************
//   Global Variables
//**********************************************************************************

u32		g_Cash=0;
u32		g_Bet=0;

s32		g_MinsPlayed;
s32		g_StartTick;

u32		g_CacheKey;

int		g_GameState;
int		g_GameCmd;
int		lastkey=0;
int		g_Debug=0;
int		g_automode=0;		

int		g_CheatCash;
int		g_StartCash;

int		g_Winner;

char	dealerdeck[52][4];
char	playerdeck[52][4];
char	playdeck[52][4];
char	decktrash[52][4];

char	decktemp[1][4];

int		DeckCards, DealerCards, PlayerCards, TrashCards;

char    deck[52][4]={"AC","2C","3C","4C","5C","6C","7C","8C","9C","10C","JC","QC","KC", 
					 "AD","2D","3D","4D","5D","6D","7D","8D","9D","10D","JD","QD","KD", 
					 "AH","2H","3H","4H","5H","6H","7H","8H","9H","10H","JH","QH","KH", 
					 "AS","2S","3S","4S","5S","6S","7S","8S","9S","10S","JS","QS","KS"};

//**********************************************************************************
//   Static Variables
//**********************************************************************************
static char		s_StringBuffer[ 1024 ];


//*************************************************************************************
// Generic Random Number Generator
//*************************************************************************************
							
// get a num rand(l,h)  (rnd % (h-1)) + l
static unsigned long next = 1;

int rand(void)
{
    next = next * 1103515245 + 12345;
    return ((unsigned int) (next / 65536UL) % 32767UL);
}

void srand(unsigned int seed)
{
    next = seed;
}


//*************************************************************************************
// ProcessGameCmd
// main game handler/renderer
//*************************************************************************************
void	ProcessGameCmd()
{
	int	playerscore;
	//u16 rgb;		
	//int r=0,g=0,b=0;
	s32 ticks;	
	u32 result32;	
	static int fps=0,lastfps=0;
	
		CacheKey();

		//rgb = (b<<10)+(g<<5)+(r<<0);		//each color is 5 bits, 32 colors
		//pgFillvram( rgb );				//test code only
		pgFillvram( 0x0000 );

		if( g_GameCmd == GS_INIT )
		{			
			g_CheatCash = 1000;
			g_StartCash = 5000;
			g_GameCmd = GS_BET;

			// cannot load for now
			g_Cash = g_StartCash;
			g_Bet = 0;
			// try to load settings
			//SettingsFile(1);

		}
		else if( g_GameCmd == GS_WELCOME )
		{


		}
		else if( g_GameCmd == GS_MENU )
		{

		}
		else if( g_GameCmd == GS_BET )
		{
			g_GameState = g_GameCmd;
			g_Winner=0;

			if( g_Bet > g_Cash )
			{
				g_Bet = g_Cash;
			}
			
			pgPrint( WINDOW_XPOS, 20, 0xffff, "Cash $", 0xFF );
			pgPrint( WINDOW_XPOS+55, 20, 0xffff, NumberToAscii( g_Cash, s_StringBuffer ), 0xFF );

			pgPrint( WINDOW_XPOS+170, 20, 0xffff, "Bet $", 0xFF );
			pgPrint( WINDOW_XPOS+215, 20, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );
			
		}
		else if( g_GameCmd == GS_STOREBET )
		{
			g_GameCmd = GS_DEAL;
		}
		else if( g_GameCmd == GS_DEAL )
		{
			g_automode = 0;
			PlayerCards=0;
			DealerCards=0;

			// Draw Cards 1=player, 2=dealer
			DrawCard(1);						
			DrawCard(2);						
			DrawCard(1);						
			DrawCard(2);

			g_GameState = g_GameCmd;
			g_GameCmd = GS_PLAYERTURN;						

			g_Cash -= g_Bet;

		}
		else if( g_GameCmd == GS_PLAYERTURN )
		{
			// always check for blackjack
			playerscore = CalcPlayer();
			if( playerscore == 21 && PlayerCards == 2)
			{
				g_GameCmd = GS_SHOWDEALER;
			}			
			else if( CalcDealer() == 21 )
			{
				// dealer has BJ
				g_GameCmd = GS_SHOWDEALER;
			}
			else if( playerscore > 21 )
			{
				pgPrint( 2, 220, 0xffff, "Player Busts!", 0xFF );
				g_GameState = GS_SHOWDEALER;				
			}
			else
			{
				g_GameState = g_GameCmd;				
			}
		}
		else if( g_GameCmd == GS_SHOWDEALER )
		{
			g_GameState = GS_DEALERTURN;
			g_GameCmd = GS_DEALERTURN;
		}
		else if( g_GameCmd == GS_DEALERTURN )
		{
			g_GameState = g_GameCmd;			
			playerscore = CalcPlayer();
			if( playerscore == 21 && PlayerCards == 2)
			{
				//skip dealers turn on bj
				g_GameState = GS_RESULTS;				
				DetermineWinner();
			}
			else
			{
				DoDealerTurn();			
			}
		}
		else if( g_GameCmd == GS_RESULTS )
		{
			g_GameState = g_GameCmd;			
			DetermineWinner();
			
		}
		else if( g_GameCmd == GS_PAYOFF )
		{
			if( g_Winner == 1 )
			{
				playerscore = CalcPlayer();
				if( playerscore == 21 && PlayerCards == 2)
				{
					result32 = g_Bet / 2;
					if( (g_Bet % 2) != 0 )
					{
						result32++;
					}
					g_Cash += g_Bet + g_Bet+ result32; // BJ pays 1.5
				}
				else
				{
					g_Cash += g_Bet + g_Bet;
				}						
			} else if( g_Winner == 0 )
			{
				g_Cash += g_Bet;
			}
			g_GameCmd = GS_BET;
		}
		else if( g_GameCmd == GS_ENDGAME )
		{

		}
		else if( g_GameCmd == GS_EXITGAME )
		{

		}
		else if( g_GameCmd == GS_REFRESH )
		{
			g_GameCmd = GS_IDLE;
		}
				
						
		DisplayDealerHand();
		DisplayPlayerHand();
		DisplayGameInstructions();

		pgPrint( WINDOW_XPOS, 5, 0xffff, "BlackJack v0.2b by GlobWare.com", 0xFF );
		if( g_Debug == 1 )
		{
			DrawDebugInfo();
		}						

		fps++;

		ticks = sceKernelLibcClock();
		ticks -= g_StartTick;
		ticks /= 1000000;   //600000 = secs, set at mins now
		if( ticks > 0 )
		{
			lastfps=fps;
			fps=0;
			g_StartTick = sceKernelLibcClock();
		}

		//pgPrint( WINDOW_XPOS+400, 2, 0xffff, NumberToAscii( sceKernelLibcClock(), s_StringBuffer ), 0xFF );
		pgPrint( WINDOW_XPOS+440, 5, 0xffff, NumberToAscii( lastfps, s_StringBuffer ), 0xFF );
		
		//ticks /= 60;	// get mins
		//pgPrint( WINDOW_XPOS+350, 2, 0xffff, "mins:", 0xFF );
		//pgPrint( WINDOW_XPOS+400, 2, 0xffff, NumberToAscii( ticks, s_StringBuffer ), 0xFF );

		CacheKey();

		pgScreenFlipV();		
		
}

//*************************************************************************************
// DisplayGameInstructions
// draws help info on screen
//*************************************************************************************
void	DisplayGameInstructions()
{
	int x;

	for(x=0;x<32;x++)
	{
		pgBitBlt(x*16,0,16,16,1,image_menubar);				
	}

	//pgBitBlt(28*16,0,16,16,1,image_anim1);				

	if( g_Debug == 1 )
	{
		for(x=0;x<29;x++)
		{
			pgBitBlt(x*16+6,141,16,16,1,image_debug);		
			pgBitBlt(x*16+6,157,16,16,1,image_debug);		
			pgBitBlt(x*16+6,173,16,16,1,image_debug);		
			pgBitBlt(x*16+6,189,16,16,1,image_debug);		
		}
	}

	// display instruction text and flip screen
	if( g_GameState <= GS_BET )
	{
		pgBitBlt(WINDOW_XPOS,251,16,16,1,image_btn_cross);
		pgPrint( WINDOW_XPOS+14,256,0xffff,"=Deal", 0xA0);

		pgBitBlt(WINDOW_XPOS+70,251,6,16,1,image_btn_left);		
		pgBitBlt(WINDOW_XPOS+80,251,6,16,1,image_btn_right);

		pgPrint( WINDOW_XPOS+90,256,0xffff,"=-/+$1", 0xA0);

		//pgBitBlt(WINDOW_XPOS+130,251,16,16,1,image_btn_circle);
				

		pgBitBlt(WINDOW_XPOS+160,251,9,8,1,image_btn_up);
		//pgPrint( WINDOW_XPOS+198,256,0xffff,"=-$100", 0xA0);

		pgBitBlt(WINDOW_XPOS+160,259,9,8,1,image_btn_down);
		pgPrint( WINDOW_XPOS+174,256,0xffff,"=-/+$100", 0xA0);

		pgPrint( WINDOW_XPOS+270, 256, 0xffff, "START=EXIT", 0x80);

	}	
	else if( g_GameState == GS_DEALERTURN || g_GameState == GS_SHOWDEALER )
	{		
		pgBitBlt(WINDOW_XPOS,251,16,16,1,image_btn_cross);
		pgPrint( WINDOW_XPOS+14,256,0xffff,"=Next", 0xA0);

		pgBitBlt(WINDOW_XPOS+70,251,16,16,1,image_btn_circle);
		pgPrint( WINDOW_XPOS+84,256,0xffff,"=Continous", 0xA0);

		pgPrint( WINDOW_XPOS+220, 256, 0xffff, "START=EXIT", 0x80);
		
	}
	else if( g_GameState > GS_DEALERTURN )
	{
		
		pgBitBlt(WINDOW_XPOS,251,16,16,1,image_btn_cross);
		pgPrint( WINDOW_XPOS+14,256,0xffff,"=RESTART", 0xA0);

		pgPrint( WINDOW_XPOS+220, 256, 0xffff, "START=EXIT", 0x80);
			
	}
	else
	{
		
		pgBitBlt(WINDOW_XPOS,251,16,16,1,image_btn_cross);
		pgPrint( WINDOW_XPOS+14,256,0xffff,"=Hit", 0xA0);

		pgBitBlt(WINDOW_XPOS+60,251,16,16,1,image_btn_circle);
		pgPrint( WINDOW_XPOS+74,256,0xffff,"=Stand", 0xA0);

		pgPrint( WINDOW_XPOS+220, 256, 0xffff, "START=EXIT", 0x80);
		
	}
}

//*************************************************************************************
// DisplayPlayerHand
// draw player hand
// place graphics here in future
//*************************************************************************************
void DisplayCard(int cardnum,int player,int x,int y)
{
	char card;	
	char face;

	if( player == 1 )
	{	
		card = playerdeck[cardnum][0];		
		if( card == '1' )
		{
			face = playerdeck[cardnum][2];
		}
		else
		{
			face = playerdeck[cardnum][1];
		}		
	}
	else if( player == 2 )
	{
		if( cardnum == 999 )	// draw back side of card to hide it
		{
			face=' ';
			card=' ';
			pgBitBlt(x,y,55,74,1,img_B);
		}
		else
		{
			card = dealerdeck[cardnum][0];		
			if( card == '1' )
			{
				face = dealerdeck[cardnum][2];
			}
			else
			{
				face = dealerdeck[cardnum][1];
			}		
		}
	}

	if( face == 'C' )
	{
		switch( card )
		{
			case 'A':
				pgBitBlt(x,y,55,74,1,img_AC);
				break;
			case '2':
				pgBitBlt(x,y,55,74,1,img_2C);
				break;
			case '3':
				pgBitBlt(x,y,55,74,1,img_3C);
				break;
			case '4':
				pgBitBlt(x,y,55,74,1,img_4C);
				break;
			case '5':
				pgBitBlt(x,y,55,74,1,img_5C);
				break;
			case '6':
				pgBitBlt(x,y,55,74,1,img_6C);
				break;
			case '7':
				pgBitBlt(x,y,55,74,1,img_7C);
				break;
			case '8':
				pgBitBlt(x,y,55,74,1,img_8C);
				break;
			case '9':
				pgBitBlt(x,y,55,74,1,img_9C);
				break;
			case '1':
				pgBitBlt(x,y,55,74,1,img_10C);
				break;
			case 'J':
				pgBitBlt(x,y,55,74,1,img_JC);
				break;
			case 'Q':
				pgBitBlt(x,y,55,74,1,img_QC);
				break;
			case 'K':
				pgBitBlt(x,y,55,74,1,img_KC);
				break;
		}		
	}
	else if( face == 'D' )
	{
		switch( card )
		{
			case 'A':
				pgBitBlt(x,y,55,74,1,img_AD);
				break;
			case '2':
				pgBitBlt(x,y,55,74,1,img_2D);
				break;
			case '3':
				pgBitBlt(x,y,55,74,1,img_3D);
				break;
			case '4':
				pgBitBlt(x,y,55,74,1,img_4D);
				break;
			case '5':
				pgBitBlt(x,y,55,74,1,img_5D);
				break;
			case '6':
				pgBitBlt(x,y,55,74,1,img_6D);
				break;
			case '7':
				pgBitBlt(x,y,55,74,1,img_7D);
				break;
			case '8':
				pgBitBlt(x,y,55,74,1,img_8D);
				break;
			case '9':
				pgBitBlt(x,y,55,74,1,img_9D);
				break;
			case '1':
				pgBitBlt(x,y,55,74,1,img_10D);
				break;
			case 'J':
				pgBitBlt(x,y,55,74,1,img_JD);
				break;
			case 'Q':
				pgBitBlt(x,y,55,74,1,img_QD);
				break;
			case 'K':
				pgBitBlt(x,y,55,74,1,img_KD);
				break;
		}		
	}
	else if( face == 'H' )
	{
		switch( card )
		{
			case 'A':
				pgBitBlt(x,y,55,74,1,img_AH);
				break;
			case '2':
				pgBitBlt(x,y,55,74,1,img_2H);
				break;
			case '3':
				pgBitBlt(x,y,55,74,1,img_3H);
				break;
			case '4':
				pgBitBlt(x,y,55,74,1,img_4H);
				break;
			case '5':
				pgBitBlt(x,y,55,74,1,img_5H);
				break;
			case '6':
				pgBitBlt(x,y,55,74,1,img_6H);
				break;
			case '7':
				pgBitBlt(x,y,55,74,1,img_7H);
				break;
			case '8':
				pgBitBlt(x,y,55,74,1,img_8H);
				break;
			case '9':
				pgBitBlt(x,y,55,74,1,img_9H);
				break;
			case '1':
				pgBitBlt(x,y,55,74,1,img_10H);
				break;
			case 'J':
				pgBitBlt(x,y,55,74,1,img_JH);
				break;
			case 'Q':
				pgBitBlt(x,y,55,74,1,img_QH);
				break;
			case 'K':
				pgBitBlt(x,y,55,74,1,img_KH);
				break;
		}		
	}
	else if( face == 'S' )
	{
		switch( card )
		{
			case 'A':
				pgBitBlt(x,y,55,74,1,img_AS);
				break;
			case '2':
				pgBitBlt(x,y,55,74,1,img_2S);
				break;
			case '3':
				pgBitBlt(x,y,55,74,1,img_3S);
				break;
			case '4':
				pgBitBlt(x,y,55,74,1,img_4S);
				break;
			case '5':
				pgBitBlt(x,y,55,74,1,img_5S);
				break;
			case '6':
				pgBitBlt(x,y,55,74,1,img_6S);
				break;
			case '7':
				pgBitBlt(x,y,55,74,1,img_7S);
				break;
			case '8':
				pgBitBlt(x,y,55,74,1,img_8S);
				break;
			case '9':
				pgBitBlt(x,y,55,74,1,img_9S);
				break;
			case '1':
				pgBitBlt(x,y,55,74,1,img_10S);
				break;
			case 'J':
				pgBitBlt(x,y,55,74,1,img_JS);
				break;
			case 'Q':
				pgBitBlt(x,y,55,74,1,img_QS);
				break;
			case 'K':
				pgBitBlt(x,y,55,74,1,img_KS);
				break;
		}		
	}

}

void	DisplayPlayerHand()
{
	int x, result;

	// display player hand
	if( g_GameState >= GS_DEAL )
	{
		pgPrint(WINDOW_XPOS,130,0xffff, "Player Hand:", 0xFF);	
		for(x=0;x<PlayerCards;x++)
		{
			DisplayCard(x,1,(x*56)+WINDOW_XPOS,140);
			//pgPrint((x*36)+WINDOW_XPOS,110,0xffff, playerdeck[x], 0xFF);	
		}
		
		pgPrint( WINDOW_XPOS, 20, 0xffff, "Cash $", 0xFF );
		pgPrint( WINDOW_XPOS+55, 20, 0xffff, NumberToAscii( g_Cash, s_StringBuffer ), 0xFF );

		pgPrint( WINDOW_XPOS+170, 20, 0xffff, "Bet $", 0xFF );
		pgPrint( WINDOW_XPOS+215, 20, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );

		result = CalcPlayer();
		pgPrint( 100+WINDOW_XPOS, 130, 0xffff, NumberToAscii( result, s_StringBuffer ), 0xFF );
	}

}

//*************************************************************************************
// DisplayDealerHand
// draw dealer hand
// place graphics here in future
//*************************************************************************************
void	DisplayDealerHand()
{
	int x, dealerscore;

	dealerscore = CalcDealer();

	// display dealer hand
	if( g_GameState <= GS_SHOWDEALER && g_GameState >= GS_DEAL )
	{
		pgPrint(WINDOW_XPOS,35,0xffff, "Dealer Hand:", 0xFF);	
		for(x=0;x<DealerCards;x++)
		{
			if( x == 0 )
			{
				//pgPrint((x*36)+WINDOW_XPOS,80,0xffff, "??", 0xFF);	
				DisplayCard(999,2,(x*56)+WINDOW_XPOS,45); // 999 = show back
			} else {
				//pgPrint((x*36)+WINDOW_XPOS,80,0xffff, dealerdeck[x], 0xFF);	
				DisplayCard(x,2,(x*56)+WINDOW_XPOS,45);
			}
		}

		// dont show card total
		pgPrint( 100+WINDOW_XPOS, 35, 0xffff, "?", 0xFF );
	}
	else if( g_GameState >= GS_DEAL )
	{
		pgPrint(WINDOW_XPOS,35,0xffff, "Dealer Hand:", 0xFF);	
		for(x=0;x<DealerCards;x++)
		{
			//pgPrint((x*36)+WINDOW_XPOS,80,0xffff, dealerdeck[x], 0xFF);	
			DisplayCard(x,2,(x*56)+WINDOW_XPOS,45);
		}
		pgPrint( 100+WINDOW_XPOS, 35, 0xffff, NumberToAscii( dealerscore, s_StringBuffer ), 0xFF );
	}
}

//*************************************************************************************
// DrawDebugInfo
// display debug window/info
//*************************************************************************************
void	DrawDebugInfo()
{
	int row, col, x;
	

	pgPrint(WINDOW_XPOS,150,0xffff, "DEBUG:", 0xFF);	
	// display entire playdeck if g_Debug is = 1		
	for(x=0;x<52;x++)
	{
		row = x / 13;
		col = x % 13;			

		if( x < DeckCards )
		{
			pgPrint((col*36)+WINDOW_XPOS,(row*10)+160,0xffff, playdeck[x], 0xB0);
		} else
		{
			pgPrint((col*36)+WINDOW_XPOS,(row*10)+160,0xffff, playdeck[x], 0x40);
		}
	}

	pgPrint( 400, 256, 0xffff, NumberToAscii( sceKernelLibcClock(), s_StringBuffer ), 0xFF );

	/*
	pgPrint( 240, 2, 0xffff, "C:",0xFF);
	pgPrint( 260, 2, 0xffff, NumberToAscii( g_GameCmd, s_StringBuffer ), 0xFF );
	pgPrint( 330, 2, 0xffff, "S:",0xFF);
	pgPrint( 350, 2, 0xffff, NumberToAscii( g_GameState, s_StringBuffer ), 0xFF );		
	

	pgPrint( WINDOW_XPOS, 2, 0xffff, "Cards:",0xFF);
	pgPrint( WINDOW_XPOS+60, 2, 0xffff, NumberToAscii( DeckCards, s_StringBuffer ), 0xFF );
	pgPrint( WINDOW_XPOS+100, 2, 0xffff, "Pile: ", 0xFF);
	pgPrint( WINDOW_XPOS+150, 2, 0xffff, NumberToAscii( TrashCards, s_StringBuffer ), 0xFF );
	*/
}

//*************************************************************************************
// DoDealerTurn
// play dealers hand in auto mode or step mode
//*************************************************************************************
void	DoDealerTurn()
{
	int result;

	result = CalcDealer();
						
	if( g_automode == 1 )
	{
		while( result < 17 )			
		{
			DrawCard(2);				
			result = CalcDealer();
		}
		g_GameCmd=GS_RESULTS;		// finished dealer turn
	}
	else
	{
		if( result >= 17 )
		{
			g_GameCmd=GS_RESULTS;		// finished dealer turn			
		}
		// dont change game state
	}
}

//*************************************************************************************
// ProcessInput
// Main Key Handler
// Uses g_GameCmd and g_GameState to determine what to do
//*************************************************************************************

void	CacheKey()
{
	u32 testkey;

	testkey = CTRL_GetKey();

	if( testkey != 0 && g_CacheKey == 0 )
	{
		g_CacheKey = testkey;
	}
	//pgPrint( 400, 256, 0xffff,NumberToAscii( testkey, s_StringBuffer ) , 0x40 );	

}

void	ProcessInput()
{
	int result,x;
	u32	testkey;

	if( g_CacheKey != 0 )
	{
		testkey = g_CacheKey;
		g_CacheKey=0;
	}
	else
	{
		testkey = CTRL_GetKey();
	}
	
		// Key Handler
		if ( testkey & KEY_START )
		{
			g_GameCmd = GS_EXITGAME;			
		}
		else if ( testkey & KEY_LEFT )
		{								
			if( lastkey != testkey )
			{
				if( g_GameState == GS_BET )
				{				
					if( g_Bet )
					{
						g_Bet--;		
					}
				}
				//allow quick scroll
				//lastkey = testkey;
			}

		}
		else if ( testkey & KEY_RIGHT )
		{			
			if( lastkey != testkey )
			{
				if( g_GameState == GS_BET )
				{
					if( g_Bet < g_Cash )
					{
						g_Bet++;		
					}
				}
				//allow quick scroll
				//lastkey = testkey;
			}

		}
		else if ( testkey & KEY_UP )
		{								
			if( lastkey != testkey )
			{
				if( g_GameState == GS_BET )
				{				
					if( g_Bet >= 100 )
					{
						g_Bet-=100;		
					}
				}
				lastkey = testkey;
			}

		}
		else if ( testkey & KEY_DOWN )
		{			
			if( lastkey != testkey )
			{
				if( g_GameState == GS_BET )
				{				
					if( g_Bet+100 <= g_Cash )
					{
						g_Bet+=100;
					}
				}
				lastkey = testkey;
			}
						
		}
		else if ( testkey & KEY_RTRIGGER )
		{
			// cheat - add $1000
			if( lastkey != KEY_RTRIGGER )
			{
				g_Cash += g_CheatCash;				
				lastkey = KEY_RTRIGGER;
			}
						
		}
		else if ( testkey & KEY_LTRIGGER )
		{
			// reset to starting cash $5000
			if( lastkey != KEY_LTRIGGER )
			{
				g_Cash = g_StartCash;				
				lastkey = KEY_LTRIGGER;
			}
						
		}

		else if ( testkey & KEY_CROSS )
		{
			if( lastkey != KEY_CROSS )
			{				
				if( g_GameState == GS_PLAYERTURN )
				{					
					// hit					
					DrawCard(1);										
				}
				else if( g_GameState == GS_BET )
				{
					if( g_Bet > 0 )
					{
						g_GameCmd = GS_STOREBET;
					}
				}
				else if( g_GameState == GS_DEALERTURN )
				{
					if( CalcDealer() < 17 )
					{
						DrawCard(2);				
						pgPrint( WINDOW_XPOS, 200, 0xffff, "Dealer Hits", 0xFF );
					} else {
						pgPrint( WINDOW_XPOS, 200, 0xffff, "Dealer Stands", 0xFF );
					}
					result = CalcDealer();
					if( result < 17 )
					{
						g_GameCmd = GS_REFRESH;
					}
					else
					{
						g_GameCmd = GS_RESULTS;
					}
				}
				else if( g_GameState == GS_RESULTS )
				{
					g_GameState = GS_INIT;
					g_GameCmd=GS_PAYOFF;		//restart game
					// move cards to trash pile
					for(x=0;x<PlayerCards;x++)
					{
						decktrash[TrashCards][0] = playerdeck[x][0];
						decktrash[TrashCards][1] = playerdeck[x][1];
						decktrash[TrashCards][2] = playerdeck[x][2];
						TrashCards++;
					}	
					for(x=0;x<DealerCards;x++)
					{
						decktrash[TrashCards][0] = dealerdeck[x][0];
						decktrash[TrashCards][1] = dealerdeck[x][1];
						decktrash[TrashCards][2] = dealerdeck[x][2];
						TrashCards++;
					}
				}
				else if( g_GameState == GS_SHOWDEALER )
				{
					g_GameCmd = GS_SHOWDEALER;
				}
				else if( g_GameState >= GS_PAYOFF )
				{
					g_GameState=GS_INIT;
					g_GameCmd=GS_BET;	// restart game
				}
				lastkey = KEY_CROSS;
			}
			
		}
		else if ( testkey & KEY_CIRCLE )
		{
			if( lastkey != KEY_CIRCLE )
			{
				if( g_GameState == GS_PLAYERTURN )
				{									
					g_GameCmd=GS_SHOWDEALER;
				} else if( g_GameState == GS_DEALERTURN || g_GameState == GS_SHOWDEALER )
				{
					g_automode = 1;
					g_GameCmd = GS_SHOWDEALER;
				} 
				lastkey=KEY_CIRCLE;
			}
		}
		/*
		else if ( testkey & KEY_SQUARE )
		{				
			// no cmds			
		}
		*/
		else if ( testkey & KEY_TRIANGLE )
		{
			if( lastkey != KEY_TRIANGLE )
			{				
				lastkey = KEY_TRIANGLE;
				g_Debug++;
				if( g_Debug == 2 ) { g_Debug=0; };
				if( g_GameCmd == GS_IDLE ) { g_GameCmd = GS_REFRESH; }
			}
		}
		else
		{
			lastkey=0;
		}						


}

// Not currently working - removed
// 1=load, 2=save
void	SettingsFile(int cmd)
{
	//char dummy[25];
	int fd;
	u32 tcash;
	u32 tbet;
	int bytes1,bytes2;

	if( cmd == 1 )
	{
		//fd = sceIoOpen("ms0:/psp/game/pspcasino/pspcasino.dat", O_RDONLY|O_NOWAIT);
		fd = sceIoOpen("ms0:/PSP/GAME/PSPBlackJack/pspblackjack.dat", O_RDONLY|O_NOWAIT);
		bytes1 = sceIoRead(fd, &tcash, 4);
		bytes2 = sceIoRead(fd, &tbet, 4);		
		sceIoClose(fd);
		if( bytes1 < 4 || bytes2 < 4 )	// if file is incomp reset it
		{
			g_Cash = g_StartCash;
			g_Bet=0;
		}
		else
		{
			g_Cash = tcash;
			g_Bet = tbet;
		}
	}
	else if( cmd == 2 )
	{
		//fd = sceIoOpen("ms0:/psp/game/TEST/debug.txt", O_CREAT|O_RDWR|O_APPEND);
		//fd = sceIoOpen("ms0:/psp/game/pspcasino/pspcasino.dat", O_CREAT|O_RDWR);
		fd = sceIoOpen("ms0:/PSP/GAME/PSPBlackJack/pspblackjack.dat", O_CREAT|O_RDWR);
		sceIoWrite(fd, &g_Cash, 4);
		sceIoWrite(fd, &g_Bet, 4);
		sceIoClose(fd);
	}
}

//*************************************************************************************
// DetermineWinner
// display winner results
//*************************************************************************************
void	DetermineWinner()
{
	int playerscore, result, dealerscore;
	u32 result32;
	
		playerscore = CalcPlayer();
		result = playerscore;
		if( playerscore > 21 )
		{
			playerscore=0;
		}

		dealerscore = CalcDealer();
		result = dealerscore;
		if( dealerscore > 21 )
		{
			dealerscore=0;
		}

		if( dealerscore > playerscore )
		{			
			if( playerscore == 0 )
			{
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Player Busted! Dealer Wins!", 0xFF );
			} else {
				if( dealerscore == 21 && DealerCards == 2 )
				{
					pgPrint( WINDOW_XPOS, 220, 0xffff, "Dealer has BlackJack!", 0xFF );
				}
				else
				{
					pgPrint( WINDOW_XPOS, 220, 0xffff, "Dealer Wins!", 0xFF );
				}
			}	

			pgPrint(WINDOW_XPOS, 230, 0xffff, "You lose $",0xFF);
			pgPrint( WINDOW_XPOS+90, 230, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );

			g_Winner=2;
		}
		else if( playerscore > dealerscore )
		{
			pgPrint(WINDOW_XPOS, 230, 0xffff, "You win $",0xFF);			
			if( playerscore == 21 && PlayerCards == 2 )
			{
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Blackjack! Player Wins!", 0xFF );
				result32 = g_Bet / 2;  // bj 1.5				
				if( (g_Bet % 2) != 0 )
				{
					result32++;
				}
				result32 += g_Bet;
				pgPrint( WINDOW_XPOS+80, 230, 0xffff, NumberToAscii( result32, s_StringBuffer ), 0xFF );
			}
			else if( dealerscore == 0 )
			{
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Dealer Busts! Player Wins!", 0xFF );
				pgPrint( WINDOW_XPOS+80, 230, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );
			} else {
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Dealer Stands. Player Wins!", 0xFF );
				pgPrint( WINDOW_XPOS+80, 230, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );
			}		
			g_Winner=1;
		}
		else
		{
			if( playerscore == 0 )
			{
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Dealer Busts! Player Busted!", 0xFF );
				g_Winner=2;	// lost money no winner, faked with 2
				pgPrint(WINDOW_XPOS, 230, 0xffff, "You lose $",0xFF);
				pgPrint( WINDOW_XPOS+90, 230, 0xffff, NumberToAscii( g_Bet, s_StringBuffer ), 0xFF );
			}
			else
			{
				pgPrint( WINDOW_XPOS, 220, 0xffff, "Tie! Player Pushes!", 0xFF );				
				g_Winner=0;
			}
		}		
}

//*************************************************************************************
// CalcPlayer
// calc players hand
//*************************************************************************************
int CalcPlayer(void)
{
	int Score=0;
	int Aces=0;
	int x=0;
	char num;
	int charnum;

	for(x=0;x<PlayerCards;x++)
	{
		num = playerdeck[x][0];	// card value				

		//if( num == 65 )
		if( num == 'A' )
		{
			Aces++;
			charnum=11;		
		}
		//else if( num == 74 || num == 81 || num == 75 )
		else if( num == 'J' || num == 'Q' || num == 'K' )
		{
			Score += 10;
			charnum=10;
		}				
		else
		{		
			num-=48;
			if( num == 1 )
			{
				num=10;				
			}		
			charnum=num;
			Score += num;		
		}		
	}

	if( Aces > 0 )
	{
		if( Aces == 2 && Score == 0 )
		{
			Score=12;
		}
		else if( Score + 11 > 21 || Aces > 2 )
		{
			do {
				Score++;
				Aces--;			
			} while( Aces > 0 );	
		}
		else
		{
			Score+=11;
			Aces--;
			if( Aces > 0 )
			{
				Score++;
				Aces--;
			}
		}
	}
	return Score;
}

//*************************************************************************************
// CalcDealer
// calc dealers hand
//*************************************************************************************
int CalcDealer(void)
{
	int Score=0;
	int Aces=0;
	int x=0;	
	char num;
	int charnum;

	for(x=0;x<DealerCards;x++)
	{
		num = dealerdeck[x][0];	// card value				

		//if( num == 65 )
		if( num == 'A' )
		{
			Aces++;
			charnum=11;
		}
		//else if( num == 74 || num == 81 || num == 75 )
		else if( num == 'J' || num == 'Q' || num == 'K' )
		{
			Score += 10;
			charnum=10;
		}				
		else
		{		
			num-=48;
			if( num == 1 )
			{
				num=10;				
			}		
			charnum=num;
			Score += num;		
		}		
	}

	if( Aces > 0 )
	{
		if( Aces == 2 && Score == 0 )
		{
			Score=12;
		}
		else if( Score + 11 > 21 || Aces > 2 )
		{
			do {
				Score++;
				Aces--;			
			} while( Aces > 0 );	
		}
		else
		{
			Score+=11;
			Aces--;
			if( Aces > 0 )
			{
				Score++;
				Aces--;
			}
		}
	}
	return Score;
}

//*************************************************************************************
// BuildDeck
// builds the play deck from the master deck
//*************************************************************************************
void BuildDeck(void)
{
	int x;

	// build deck from master deck
	for( x=0;x<52;x++)
	{		
		playdeck[x][0] = deck[x][0];
		playdeck[x][1] = deck[x][1];
		playdeck[x][2] = deck[x][2];

	}
	DeckCards = 52;
	TrashCards=0;
	PlayerCards=0;
	DealerCards=0;
}

//*************************************************************************************
// DrawCard
// grabs next avail card from play deck
// will reshuffle trashcards (pile) back
// into play deck if necessary.
//*************************************************************************************
void DrawCard(int player)
{
	int x, num, trashcnt;

	if( DeckCards == 0 )
	{
		// fill play deck with trash pile
		trashcnt = TrashCards;
		for(x=0;x<trashcnt;x++)
		{
			playdeck[DeckCards][0] = decktrash[x][0];
			playdeck[DeckCards][1] = decktrash[x][1];
			playdeck[DeckCards][2] = decktrash[x][2];
			DeckCards++;
			TrashCards--;
		}
		// need to reshuffle 5x
		for(x=0;x<5;x++)
		{
			PlayDeckShuffle();
		}
		// Below is to add cards in player hand back into the
		// play deck to draw on the debug screen only.
		// when re-building a new playdeck cards in player & dealer
		// hand will not be drawn in the original draw order.
		// shortcut taken since it's for debug only
		num=0;
		for(x=0;x<PlayerCards;x++)
		{			
			playdeck[DeckCards+num][0] = playerdeck[x][0];
			playdeck[DeckCards+num][1] = playerdeck[x][1];
			playdeck[DeckCards+num][2] = playerdeck[x][2];
			num++;
		}
		for(x=0;x<DealerCards;x++)
		{			
			playdeck[DeckCards+num][0] = dealerdeck[x][0];
			playdeck[DeckCards+num][1] = dealerdeck[x][1];
			playdeck[DeckCards+num][2] = dealerdeck[x][2];
			num++;
		}
	}
	
	// moves card pointer to next avail card - do first since
	// ptr starts at 52 not 51 :)
	DeckCards--;	

	// grab card from play deck and play in players hand
	// 1=player, 2=dealer
	if( player == 1 )
	{
		playerdeck[PlayerCards][0] = playdeck[DeckCards][0];
		playerdeck[PlayerCards][1] = playdeck[DeckCards][1];
		playerdeck[PlayerCards][2] = playdeck[DeckCards][2];
		PlayerCards++;
	} 
	else if( player == 2 )
	{
		dealerdeck[DealerCards][0] = playdeck[DeckCards][0];
		dealerdeck[DealerCards][1] = playdeck[DeckCards][1];
		dealerdeck[DealerCards][2] = playdeck[DeckCards][2];
		DealerCards++;
	}	
}

//*************************************************************************************
// PlayDeckShuffle
// reshuffle the play deck - when were out of cards yet cards
// are still in play by player and dealer.
//*************************************************************************************
void PlayDeckShuffle(void)
{
	int x, num;

	for( x=0;x<DeckCards;x++)
	{
		num = rand();
		num = (num % (52-1));			
		decktemp[0][0] = playdeck[x][0];
		decktemp[0][1] = playdeck[x][1];
		decktemp[0][2] = playdeck[x][2];

		playdeck[x][0] = playdeck[num][0];
		playdeck[x][1] = playdeck[num][1];
		playdeck[x][2] = playdeck[num][2];

		playdeck[num][0] = decktemp[0][0];
		playdeck[num][1] = decktemp[0][1];
		playdeck[num][2] = decktemp[0][2];
	}

}

//*************************************************************************************
// MasterShuffle
// first time shuffle with all cards
//*************************************************************************************
void MasterShuffle(void)
{
	int x, num;

	// swap up cards
	for( x=0;x<52;x++)
	{
		num = rand();
		num = (num % (52-1));			
		decktemp[0][0] = playdeck[x][0];
		decktemp[0][1] = playdeck[x][1];
		decktemp[0][2] = playdeck[x][2];

		playdeck[x][0] = playdeck[num][0];
		playdeck[x][1] = playdeck[num][1];
		playdeck[x][2] = playdeck[num][2];

		playdeck[num][0] = decktemp[0][0];
		playdeck[num][1] = decktemp[0][1];
		playdeck[num][2] = decktemp[0][2];
	}

}

//*************************************************************************************
// xmain
// main looped routine
//*************************************************************************************
int		xmain()
{
	//s32		fps, last_frame;

	g_StartTick = sceKernelLibcClock();

	g_GameState=0;
	g_GameCmd=0;

	srand(sceKernelLibcClock());

	pgInit();
	pgScreenFrame( 2, 0 );

	pgFillvram( 0x0000 );
	pgScreenFlipV();
	pgFillvram( 0x0000 );
	pgScreenFlipV();
	
	//last_frame = fps = sceKernelLibcClock();
	
	BuildDeck();
	MasterShuffle(); // reshuffle 3 times
	MasterShuffle();
	MasterShuffle();

	// fake BJ
	//playdeck[51][0]=deck[0][0];
	//playdeck[51][1]=deck[0][1];
	//playdeck[49][0]=deck[11][0];
	//playdeck[49][1]=deck[11][1];	

	//playdeck[50][0]=deck[0][0];
	//playdeck[50][1]=deck[0][1];
	//playdeck[48][0]=deck[11][0];
	//playdeck[48][1]=deck[11][1];	
	
				
	g_GameState=GS_INIT;	
	g_GameCmd = GS_INIT;

	g_StartTick = sceKernelLibcClock();

	while ( 1 )
	{
		ProcessGameCmd();
		ProcessInput();

		if( g_GameCmd == GS_EXITGAME )
		{
			// save settings first
			//SettingsFile(2);
			break;
		}
	}

	return 0;
}

//*************************************************************************************
// pgExit
// Exit Routine	
//*************************************************************************************
void pgExit( int n )
{
	__exit();
}

//*************************************************************************************
// pgMain
//Main Starter 
//*************************************************************************************
void	pgMain( unsigned long args, void * argp )
{
	int ret = xmain();
	pgExit( ret );
}

//*******************************  END OF FILE  ************************************
