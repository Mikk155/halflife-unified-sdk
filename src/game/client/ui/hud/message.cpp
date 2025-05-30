/***
 *
 *    Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *    This product contains software technology licensed from Id
 *    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *    All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
//
// Message.cpp
//
// implementation of CHudMessage class
//

#include "hud.h"

const char* g_pCustomName = "Custom";
char g_pCustomText[1024];

constexpr std::string_view NetMessagePrefix{"__NETMESSAGE__"sv};

bool CHudMessage::Init()
{
    m_CustomMessageText = g_ConCommands.CreateCVar( "custom_message_text", "" );
    m_CustomMessageX = g_ConCommands.CreateCVar( "custom_message_x", "0.1" );
    m_CustomMessageY = g_ConCommands.CreateCVar( "custom_message_y", "-1" );

    g_ClientUserMessages.RegisterHandler( "HudText", &CHudMessage::MsgFunc_HudText, this );
    g_ClientUserMessages.RegisterHandler( "GameTitle", &CHudMessage::MsgFunc_GameTitle, this );

    gHUD.AddHudElem( this );

    Reset();

    // Always run so custom messages can draw.
    m_iFlags |= HUD_ACTIVE;

    return true;
}

bool CHudMessage::VidInit()
{
    m_HUD_title_halflife.Left = gHUD.GetSpriteIndex( "title_half" );
    m_HUD_title_halflife.Right = gHUD.GetSpriteIndex( "title_life" );

    m_HUD_title_opposingforce.Left = gHUD.GetSpriteIndex( "title_opposing" );
    m_HUD_title_opposingforce.Right = gHUD.GetSpriteIndex( "title_force" );

    m_HUD_title_blueshift.Left = gHUD.GetSpriteIndex( "title_blue" );
    m_HUD_title_blueshift.Right = gHUD.GetSpriteIndex( "title_shift" );

    return true;
}


void CHudMessage::Reset()
{
    memset( m_pMessages, 0, sizeof( m_pMessages[0] ) * maxHUDMessages );
    memset( m_startTime, 0, sizeof( m_startTime[0] ) * maxHUDMessages );

    m_gameTitleTime = 0;
    m_pGameTitle = nullptr;
    m_TitleToDisplay = nullptr;
}


float CHudMessage::FadeBlend( float fadein, float fadeout, float hold, float localTime )
{
    float fadeTime = fadein + hold;
    float fadeBlend;

    if( localTime < 0 )
        return 0;

    if( localTime < fadein )
    {
        fadeBlend = 1 - ( ( fadein - localTime ) / fadein );
    }
    else if( localTime > fadeTime )
    {
        if( fadeout > 0 )
            fadeBlend = 1 - ( ( localTime - fadeTime ) / fadeout );
        else
            fadeBlend = 0;
    }
    else
        fadeBlend = 1;

    return fadeBlend;
}


int CHudMessage::XPosition( float x, int width, int totalWidth )
{
    int xPos;

    if( x == -1 )
    {
        xPos = ( ScreenWidth - width ) / 2;
    }
    else
    {
        if( x < 0 )
            xPos = ( 1.0 + x ) * ScreenWidth - totalWidth; // Alight right
        else
            xPos = x * ScreenWidth;
    }

    if( xPos + width > ScreenWidth )
        xPos = ScreenWidth - width;
    else if( xPos < 0 )
        xPos = 0;

    return xPos;
}


int CHudMessage::YPosition( float y, int height )
{
    int yPos;

    if( y == -1 ) // Centered?
        yPos = ( ScreenHeight - height ) * 0.5;
    else
    {
        // Alight bottom?
        if( y < 0 )
            yPos = ( 1.0 + y ) * ScreenHeight - height; // Alight bottom
        else                                          // align top
            yPos = y * ScreenHeight;
    }

    if( yPos + height > ScreenHeight )
        yPos = ScreenHeight - height;
    else if( yPos < 0 )
        yPos = 0;

    return yPos;
}


void CHudMessage::MessageScanNextChar()
{
    int srcRed, srcGreen, srcBlue, destRed, destGreen, destBlue;
    int blend;

    srcRed = m_parms.pMessage->r1;
    srcGreen = m_parms.pMessage->g1;
    srcBlue = m_parms.pMessage->b1;
    blend = 0; // Pure source
    destRed = destGreen = destBlue = 0;

    switch ( m_parms.pMessage->effect )
    {
        // Fade-in / Fade-out
    case 0:
    case 1:
        blend = m_parms.fadeBlend;
        break;

    case 2:
        m_parms.charTime += m_parms.pMessage->fadein;
        if( m_parms.charTime > m_parms.time )
        {
            srcRed = srcGreen = srcBlue = 0;
            blend = 0; // pure source
        }
        else
        {
            float deltaTime = m_parms.time - m_parms.charTime;

            if( m_parms.time > m_parms.fadeTime )
            {
                blend = m_parms.fadeBlend;
            }
            else if( deltaTime > m_parms.pMessage->fxtime )
                blend = 0; // pure dest
            else
            {
                destRed = m_parms.pMessage->r2;
                destGreen = m_parms.pMessage->g2;
                destBlue = m_parms.pMessage->b2;
                blend = 255 - ( deltaTime * ( 1.0 / m_parms.pMessage->fxtime ) * 255.0 + 0.5 );
            }
        }
        break;
    }
    if( blend > 255 )
        blend = 255;
    else if( blend < 0 )
        blend = 0;

    m_parms.r = ( ( srcRed * ( 255 - blend ) ) + ( destRed * blend ) ) >> 8;
    m_parms.g = ( ( srcGreen * ( 255 - blend ) ) + ( destGreen * blend ) ) >> 8;
    m_parms.b = ( ( srcBlue * ( 255 - blend ) ) + ( destBlue * blend ) ) >> 8;

    if( m_parms.pMessage->effect == 1 && m_parms.charTime != 0 )
    {
        if( m_parms.x >= 0 && m_parms.y >= 0 && ( m_parms.x + gHUD.m_scrinfo.charWidths[m_parms.text] ) <= ScreenWidth )
            TextMessageDrawChar( m_parms.x, m_parms.y, m_parms.text, m_parms.pMessage->r2, m_parms.pMessage->g2, m_parms.pMessage->b2 );
    }
}


void CHudMessage::MessageScanStart()
{
    switch ( m_parms.pMessage->effect )
    {
        // Fade-in / out with flicker
    case 1:
    case 0:
        m_parms.fadeTime = m_parms.pMessage->fadein + m_parms.pMessage->holdtime;


        if( m_parms.time < m_parms.pMessage->fadein )
        {
            m_parms.fadeBlend = ( ( m_parms.pMessage->fadein - m_parms.time ) * ( 1.0 / m_parms.pMessage->fadein ) * 255 );
        }
        else if( m_parms.time > m_parms.fadeTime )
        {
            if( m_parms.pMessage->fadeout > 0 )
                m_parms.fadeBlend = ( ( ( m_parms.time - m_parms.fadeTime ) / m_parms.pMessage->fadeout ) * 255 );
            else
                m_parms.fadeBlend = 255; // Pure dest (off)
        }
        else
            m_parms.fadeBlend = 0; // Pure source (on)
        m_parms.charTime = 0;

        if( m_parms.pMessage->effect == 1 && ( rand() % 100 ) < 10 )
            m_parms.charTime = 1;
        break;

    case 2:
        m_parms.fadeTime = ( m_parms.pMessage->fadein * m_parms.length ) + m_parms.pMessage->holdtime;

        if( m_parms.time > m_parms.fadeTime && m_parms.pMessage->fadeout > 0 )
            m_parms.fadeBlend = ( ( ( m_parms.time - m_parms.fadeTime ) / m_parms.pMessage->fadeout ) * 255 );
        else
            m_parms.fadeBlend = 0;
        break;
    }
}


void CHudMessage::MessageDrawScan( client_textmessage_t* pMessage, float time )
{
    int i, length, width;
    const char* pText;
    eastl::fixed_string<char, MAX_HUDMSG_TEXT_LENGTH> line;

    pText = pMessage->pMessage;
    // Count lines
    m_parms.lines = 1;
    m_parms.time = time;
    m_parms.pMessage = pMessage;
    length = 0;
    width = 0;
    m_parms.totalWidth = 0;
    while( '\0' != *pText )
    {
        if( *pText == '\n' )
        {
            m_parms.lines++;
            if( width > m_parms.totalWidth )
                m_parms.totalWidth = width;
            width = 0;
        }
        else
            width += gHUD.m_scrinfo.charWidths[*pText];
        pText++;
        length++;
    }
    m_parms.length = length;
    m_parms.totalHeight = ( m_parms.lines * gHUD.m_scrinfo.iCharHeight );


    m_parms.y = YPosition( pMessage->y, m_parms.totalHeight );
    pText = pMessage->pMessage;

    m_parms.charTime = 0;

    MessageScanStart();

    for( i = 0; i < m_parms.lines; i++ )
    {
        line.clear();
        m_parms.width = 0;
        while( '\0' != *pText && *pText != '\n' )
        {
            unsigned char c = *pText;
            line.push_back(c);
            m_parms.width += gHUD.m_scrinfo.charWidths[c];
            pText++;
        }
        pText++; // Skip LF

        m_parms.x = XPosition( pMessage->x, m_parms.width, m_parms.totalWidth );

        for( std::size_t j = 0; j < line.size(); j++ )
        {
            m_parms.text = line[j];
            int next = m_parms.x + gHUD.m_scrinfo.charWidths[m_parms.text];
            MessageScanNextChar();

            if( m_parms.x >= 0 && m_parms.y >= 0 && next <= ScreenWidth )
                TextMessageDrawChar( m_parms.x, m_parms.y, m_parms.text, m_parms.r, m_parms.g, m_parms.b );
            m_parms.x = next;
        }

        m_parms.y += gHUD.m_scrinfo.iCharHeight;
    }
}


bool CHudMessage::Draw( float fTime )
{
    int i;
    client_textmessage_t* pMessage;
    float endTime;

    if( m_gameTitleTime > 0 )
    {
        float localTime = gHUD.m_flTime - m_gameTitleTime;
        float brightness;

        // Maybe timer isn't set yet
        if( m_gameTitleTime > gHUD.m_flTime )
            m_gameTitleTime = gHUD.m_flTime;

        if( localTime > ( m_pGameTitle->fadein + m_pGameTitle->holdtime + m_pGameTitle->fadeout ) )
            m_gameTitleTime = 0;
        else
        {
            brightness = FadeBlend( m_pGameTitle->fadein, m_pGameTitle->fadeout, m_pGameTitle->holdtime, localTime );

            int halfWidth = gHUD.GetSpriteRect( m_TitleToDisplay->Left ).right - gHUD.GetSpriteRect( m_TitleToDisplay->Left ).left;
            int fullWidth = halfWidth + gHUD.GetSpriteRect( m_TitleToDisplay->Right ).right - gHUD.GetSpriteRect( m_TitleToDisplay->Right ).left;
            int fullHeight = gHUD.GetSpriteRect( m_TitleToDisplay->Left ).bottom - gHUD.GetSpriteRect( m_TitleToDisplay->Left ).top;

            int x = XPosition( m_pGameTitle->x, fullWidth, fullWidth );
            int y = YPosition( m_pGameTitle->y, fullHeight );

            const auto color = RGB24{m_pGameTitle->r1, m_pGameTitle->g1, m_pGameTitle->b1}.Scale( brightness * 255 );

            SPR_Set( gHUD.GetSprite( m_TitleToDisplay->Left ), color );
            SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect( m_TitleToDisplay->Left ) );

            SPR_Set( gHUD.GetSprite( m_TitleToDisplay->Right ), color );
            SPR_DrawAdditive( 0, x + halfWidth, y, &gHUD.GetSpriteRect( m_TitleToDisplay->Right ) );
        }
    }
    // Fixup level transitions
    for( i = 0; i < maxHUDMessages; i++ )
    {
        // Assume m_parms.time contains last time
        if( m_pMessages[i] )
        {
            pMessage = m_pMessages[i];
            if( m_startTime[i] > gHUD.m_flTime )
                m_startTime[i] = gHUD.m_flTime + m_parms.time - m_startTime[i] + 0.2; // Server takes 0.2 seconds to spawn, adjust for this
        }
    }

    for( i = 0; i < maxHUDMessages; i++ )
    {
        if( m_pMessages[i] )
        {
            pMessage = m_pMessages[i];

            // This is when the message is over
            switch ( pMessage->effect )
            {
            default:
            case 0:
            case 1:
                endTime = m_startTime[i] + pMessage->fadein + pMessage->fadeout + pMessage->holdtime;
                break;

                // Fade in is per character in scanning messages
            case 2:
                endTime = m_startTime[i] + ( pMessage->fadein * strlen( pMessage->pMessage ) ) + pMessage->fadeout + pMessage->holdtime;
                break;
            }

            if( fTime <= endTime )
            {
                float messageTime = fTime - m_startTime[i];

                // Draw the message
                // effect 0 is fade in/fade out
                // effect 1 is flickery credits
                // effect 2 is write out (training room)
                MessageDrawScan( pMessage, messageTime );
            }
            else
            {
                // The message is over
                m_pMessages[i] = nullptr;
            }
        }
    }

    if( m_CustomMessageText->string[0] != '\0' )
    {
        client_textmessage_t customMessage{
            .effect = 0,
            .r1 = 255,
            .g1 = 255,
            .b1 = 255,
            .a1 = 0,
            .r2 = 0,
            .g2 = 0,
            .b2 = 0,
            .a2 = 0,
            .x = m_CustomMessageX->value,
            .y = m_CustomMessageY->value,
            .fadein = 0,
            .fadeout = 0,
            .holdtime = 0,
            .pName = "#CustomMessage",
            .pMessage = m_CustomMessageText->string};

        MessageDrawScan( &customMessage, 0 );
    }

    // Remember the time -- to fix up level transitions
    m_parms.time = gHUD.m_flTime;

    return true;
}


void CHudMessage::MessageAdd( const char* pName, float time )
{
    client_textmessage_t* tempMessage;

    // Trim off a leading # if it's there
    if( pName[0] == '#' )
        tempMessage = gHUD.m_TextMessage.Getmessage( pName + 1 );
    else
        tempMessage = gHUD.m_TextMessage.Getmessage( pName );

    // If we couldnt find it in the titles.txt, just create it
    if( !tempMessage )
    {
        gHUD.m_TextMessage.pMessage.effect = 2;
        gHUD.m_TextMessage.pMessage.r1 =
            gHUD.m_TextMessage.pMessage.g1 =
                gHUD.m_TextMessage.pMessage.b1 =
                    gHUD.m_TextMessage.pMessage.a1 = 100;
        gHUD.m_TextMessage.pMessage.r2 = 240;
        gHUD.m_TextMessage.pMessage.g2 = 110;
        gHUD.m_TextMessage.pMessage.b2 = 0;
        gHUD.m_TextMessage.pMessage.a2 = 0;
        gHUD.m_TextMessage.pMessage.x = -1; // Centered
        gHUD.m_TextMessage.pMessage.y = 0.7;
        gHUD.m_TextMessage.pMessage.fadein = 0.01;
        gHUD.m_TextMessage.pMessage.fadeout = 1.5;
        gHUD.m_TextMessage.pMessage.fxtime = 0.25;
        gHUD.m_TextMessage.pMessage.holdtime = 5;
        gHUD.m_TextMessage.pMessage.pName = g_pCustomName;
        strcpy( g_pCustomText, pName );
        gHUD.m_TextMessage.pMessage.pMessage = g_pCustomText;

        tempMessage = &gHUD.m_TextMessage.pMessage;
    }

    for( int i = 0; i < maxHUDMessages; i++ )
    {
        // If this is a net message (game_text or plugin) then the same object is overwritten,
        // so clear out the original message to reuse it.
        // This fixes certain effects not resetting correctly.
        if( tempMessage == m_pMessages[i] &&
            strncmp( tempMessage->pName, NetMessagePrefix.data(), NetMessagePrefix.size() ) == 0 )
        {
            m_pMessages[i] = nullptr;
        }

        if( !m_pMessages[i] )
        {
            for( int j = 0; j < maxHUDMessages; j++ )
            {
                if( m_pMessages[j] )
                {
                    // is this message already in the list
                    if( 0 == strcmp( tempMessage->pMessage, m_pMessages[j]->pMessage ) )
                    {
                        return;
                    }

                    // get rid of any other messages in same location (only one displays at a time)
                    if( fabs( tempMessage->y - m_pMessages[j]->y ) < 0.0001 )
                    {
                        if( fabs( tempMessage->x - m_pMessages[j]->x ) < 0.0001 )
                        {
                            m_pMessages[j] = nullptr;
                        }
                    }
                }
            }

            m_pMessages[i] = tempMessage;
            m_startTime[i] = time;
            return;
        }
    }
}


void CHudMessage::MsgFunc_HudText( const char* pszName, BufferReader& reader )
{
    char* pString = reader.ReadString();

    MessageAdd( pString, gHUD.m_flTime );
    // Remember the time -- to fix up level transitions
    m_parms.time = gHUD.m_flTime;
}


void CHudMessage::MsgFunc_GameTitle( const char* pszName, BufferReader& reader )
{
    // Pick the right title.
    // This is a temporary hack. The UI needs to be rewritten so this supports the official games and uses the HL1 title for everything else.
    const std::string gameName = reader.ReadString();

    std::string titleName = gameName;

    ToUpper( titleName );

    titleName += "_GAMETITLE";

    m_pGameTitle = gHUD.m_TextMessage.Getmessage( titleName.c_str() );

    if( !m_pGameTitle )
    {
        // Fallback.
        m_pGameTitle = gHUD.m_TextMessage.Getmessage( "VALVE_GAMETITLE" );
    }

    if( m_pGameTitle != nullptr )
    {
        if( 0 == strcmp( "gearbox", gameName.c_str() ) )
        {
            m_TitleToDisplay = &m_HUD_title_opposingforce;
        }
        else if( 0 == strcmp( "bshift", gameName.c_str() ) )
        {
            m_TitleToDisplay = &m_HUD_title_blueshift;
        }
        else
        {
            m_TitleToDisplay = &m_HUD_title_halflife;
        }

        m_gameTitleTime = gHUD.m_flTime;
    }
}

void CHudMessage::MessageAdd( client_textmessage_t* newMessage )
{
    m_parms.time = gHUD.m_flTime;

    for( int i = 0; i < maxHUDMessages; i++ )
    {
        if( !m_pMessages[i] )
        {
            m_pMessages[i] = newMessage;
            m_startTime[i] = gHUD.m_flTime;
            return;
        }
    }
}
