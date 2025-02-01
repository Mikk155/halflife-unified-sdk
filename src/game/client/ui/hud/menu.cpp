/***
 *
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
//
// menu.cpp
//
// generic menu handler
//

#include <string>

#include "hud.h"

#include "vgui_TeamFortressViewport.h"

#define MAX_MENU_STRING 512
char g_szMenuString[MAX_MENU_STRING];
char g_szPrelocalisedMenuString[MAX_MENU_STRING];

std::string KB_ConvertString( const char* in );

bool CHudMenu::Init()
{
    gHUD.AddHudElem( this );

    g_ClientUserMessages.RegisterHandler( "ShowMenu", &CHudMenu::MsgFunc_ShowMenu, this );

    InitHUDData();

    return true;
}

void CHudMenu::InitHUDData()
{
    m_fMenuDisplayed = false;
    m_bitsValidSlots = 0;
    Reset();
}

void CHudMenu::Reset()
{
    g_szPrelocalisedMenuString[0] = 0;
    m_fWaitingForMore = false;
}

bool CHudMenu::VidInit()
{
    return true;
}


/*=================================
  ParseEscapeToken

  Interprets the given escape token (backslash followed by a letter). The
  first character of the token must be a backslash.  The second character
  specifies the operation to perform:

   \w : White text (this is the default)
   \d : Dim (gray) text
   \y : Yellow text
   \r : Red text
   \R : Right-align (just for the remainder of the current line)
=================================*/

static RGB24 menu_color;
static int menu_x;
static bool menu_ralign = false;

static inline const char* ParseEscapeToken( const char* token )
{
    if( *token != '\\' )
        return token;

    token++;

    switch ( *token )
    {
    case '\0':
        return token;

    case 'w':
        menu_color = {255, 255, 255};
        break;

    case 'd':
        menu_color = {100, 100, 100};
        break;

    case 'y':
        menu_color = {255, 210, 64};
        break;

    case 'r':
        menu_color = {210, 24, 0};
        break;

    case 'R':
        menu_x = ScreenWidth / 2;
        menu_ralign = true;
        break;
    }

    return ++token;
}


bool CHudMenu::Draw( float flTime )
{
	// check for if menu is set to disappear
    if( m_flShutoffTime > 0 )
    {
        if( m_flShutoffTime <= gHUD.m_flTime )
        { // times up, shutoff
            m_fMenuDisplayed = false;
            m_iFlags &= ~HUD_ACTIVE;
            return true;
        }
    }

	// don't draw the menu if the scoreboard is being shown
    if( gHUD.m_Scoreboard.m_iShowscoresHeld )
    {
        return true;
    }

	/*
	if ( gViewPort && gViewPort->IsScoreBoardVisible() )
		return true;
		*/

	// draw the menu, along the left-hand side of the screen

	// count the number of newlines
    int nlc = 0;
    int i;
    for( i = 0; i < MAX_MENU_STRING && g_szMenuString[i] != '\0'; i++ )
    {
        if( g_szMenuString[i] == '\n' )
            nlc++;
    }

	// center it
    int y = ( ScreenHeight / 2 ) - ( ( nlc / 2 ) * 12 ) - 40; // make sure it is above the say text

    menu_color = {255, 255, 255};
    menu_x = 20;
    menu_ralign = false;

    const char* sptr = g_szMenuString;

    while( *sptr != '\0' )
    {
        if( *sptr == '\\' )
        {
            sptr = ParseEscapeToken( sptr );
        }
        else if( *sptr == '\n' )
        {
            menu_ralign = false;
            menu_x = 20;
            y += ( 12 );

            sptr++;
        }
        else
        {
            char menubuf[80];
            const char* ptr = sptr;
            while( *sptr != '\0' && *sptr != '\n' && *sptr != '\\' )
            {
                sptr++;
            }
            strncpy( menubuf, ptr, std::min( ( sptr - ptr ), (int)sizeof( menubuf ) ) );
            menubuf[std::min( ( sptr - ptr ), (int)( sizeof( menubuf ) - 1 ) )] = '\0';

            if( menu_ralign )
            {
				// IMPORTANT: Right-to-left rendered text does not parse escape tokens!
                menu_x = gHUD.DrawHudStringReverse( menu_x, y, 0, menubuf, menu_color );
            }
            else
            {
                menu_x = gHUD.DrawHudString( menu_x, y, 320, menubuf, menu_color );
            }
        }
    }

    return true;
}

// selects an item from the menu
void CHudMenu::SelectMenuItem( int menu_item )
{
	// if menu_item is in a valid slot,  send a menuselect command to the server
    if( ( menu_item > 0 ) && ( m_bitsValidSlots & ( 1 << ( menu_item - 1 ) ) ) != 0 )
    {
        char szbuf[32];
        sprintf( szbuf, "menuselect %d\n", menu_item );
        EngineClientCmd( szbuf );

		// remove the menu
        m_fMenuDisplayed = false;
        m_iFlags &= ~HUD_ACTIVE;
    }
}


// Message handler for ShowMenu message
// takes four values:
//		short: a bitfield of keys that are valid input
//		char : the duration, in seconds, the menu should stay up. -1 means is stays until something is chosen.
//		byte : a boolean, true if there is more string yet to be received before displaying the menu, false if it's the last string
//		string: menu string to display
// if this message is never received, then scores will simply be the combined totals of the players.
void CHudMenu::MsgFunc_ShowMenu( const char* pszName, BufferReader& reader )
{
    m_bitsValidSlots = reader.ReadShort();
    int DisplayTime = reader.ReadChar();
    bool NeedMore = reader.ReadByte() != 0;

    if( DisplayTime > 0 )
        m_flShutoffTime = DisplayTime + gHUD.m_flTime;
    else
        m_flShutoffTime = -1;

    if( 0 != m_bitsValidSlots )
    {
        if( !m_fWaitingForMore ) // this is the start of a new menu
        {
            strncpy( g_szPrelocalisedMenuString, reader.ReadString(), MAX_MENU_STRING );
        }
        else
        { // append to the current menu string
            strncat( g_szPrelocalisedMenuString, reader.ReadString(), MAX_MENU_STRING - strlen( g_szPrelocalisedMenuString ) );
        }
        g_szPrelocalisedMenuString[MAX_MENU_STRING - 1] = 0; // ensure null termination (strncat/strncpy does not)

        if( !NeedMore )
        { // we have the whole string, so we can localise it now
            strcpy( g_szMenuString, gHUD.m_TextMessage.BufferedLocaliseTextString( g_szPrelocalisedMenuString ) );

			// Swap in characters
			// TODO: bounds check
            const auto temp = KB_ConvertString( g_szMenuString );
            strcpy( g_szMenuString, temp.c_str() );
        }

        m_fMenuDisplayed = true;
        m_iFlags |= HUD_ACTIVE;
    }
    else
    {
        m_fMenuDisplayed = false; // no valid slots means that the menu should be turned off
        m_iFlags &= ~HUD_ACTIVE;
    }

    m_fWaitingForMore = NeedMore;
}
