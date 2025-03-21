/***
 *
 *    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "InformationWindow.h"

#ifndef CLIENT_DLL
#include "UserMessages.h"
#else
#include "networking/ClientUserMessages.h"
#endif

bool CInformationWindow::Initialize()
{
    /*
        En consola:
        cl_log_setlevel test debug
        sv_log_setlevel test debug
    */
    m_Logger = g_Logging.CreateLogger( "test" );
#ifdef CLIENT_DLL
    m_window_pages.resize(MAX_PAGES);
    g_ClientUserMessages.RegisterHandler( "InformationWindow", &CInformationWindow::MsgFunc_WindowInformation, this );
#endif

    return true;
}

const int CInformationWindow::ClampPages( int page )
{
    return std::min( std::max( 0, page ), MAX_PAGES ); // Should we really limit it?
}

#ifdef CLIENT_DLL
void CInformationWindow::OpenWindow( int page )
{
    window_data& data = m_window_pages.at( page );

    if( !data.active )
    {
        m_Logger->debug( "Uhh window_data ref fue nula? pagina {}", page );
        return;
    }

    std::string& title = data.title;
    std::string& body = data.body;

    m_Logger->debug( "Se abrio el menu de la pagina {} \"{}\"", page, title );

    // @Gaftherman
}

void CInformationWindow::MsgFunc_WindowInformation( BufferReader& reader )
{
    int task = reader.ReadByte();

    int page = reader.ReadByte();

    m_Logger->debug( "Leyendo tarea {} en la pagina {}", task, page );

    window_data& data = m_window_pages.at( page );

    switch( task )
    {
        case window_sending::Cleanup:
        {
            data.active = false;
            data.title = "";
            data.body = "";
            break;
        }
        case window_sending::Title:
        {
            data.title = std::string( reader.ReadString() );
            m_Logger->debug( "Titulo {}", data.title );
            break;
        }
        case window_sending::Appending:
        {
            data.body += std::string( reader.ReadString() );
            m_Logger->debug( "Cuerpo: {}", data.body );
            break;
        }
        case window_sending::Finish:
        case window_sending::JustOpen:
        {
            // Read the last characters and open
            if( task == window_sending::Finish )
            {
                data.body += std::string( reader.ReadString() );
            }
            data.active = true;
            OpenWindow( page );
            break;
        }
    }
}
#endif

#ifndef CLIENT_DLL
const char* CInformationWindow::GetBodyString( const char* str )
{
    // Is the body a path to a .txt file?
    if( size_t len = std::strlen( str ); len >= 4 && std::strcmp( str + len - 4, ".txt" ) == 0 )
    {
        m_Logger->debug( "GetBodyString es un archivo de texto: {}", str );
        const auto fileContents = FileSystem_LoadFileIntoBuffer( str, FileContentFormat::Text );
        const char* const aFileList = reinterpret_cast<const char*>( fileContents.data() );
        return aFileList;
    }
    m_Logger->debug( "GetBodyString es un string", str );
    return str;
}

void CInformationWindow::ClearInformation( CBasePlayer* player, int page )
{
    if( player != nullptr )
    {
        m_Logger->debug( "Limpiando pagina {}", page );
        MESSAGE_BEGIN( MSG_ONE, gmsgInformationWindow, nullptr, player );
            WRITE_BYTE( ClampPages(page) );
            WRITE_BYTE( window_sending::Cleanup );
        MESSAGE_END();
    }
}

#define MAX_WINDOW_INFORMATION_CHUNK 60
#define MAX_WINDOW_INFORMATION_LENGTH 1536 // (MAX_WINDOW_INFORMATION_CHUNK * 4)

void CInformationWindow::SendInformation( CBasePlayer* player, int page, const char* title, const char* body )
{
    if( player = nullptr )
        return;

    m_Logger->debug( "Enviando pagina {} con titulo \"{}\"", page, title );

    page = ClampPages(page);

    ClearInformation(player, page);

    MESSAGE_BEGIN( MSG_ONE, gmsgInformationWindow, nullptr, player );
        WRITE_BYTE( page );
        WRITE_BYTE( window_sending::Title );
        WRITE_STRING( title );
    MESSAGE_END();

    int char_count = 0;

    const char* pBodyEntry = GetBodyString( body );

    // Send the message of the day
    // read it chunk-by-chunk,  and send it in parts
    while( pBodyEntry && '\0' != *pBodyEntry && char_count < MAX_WINDOW_INFORMATION_LENGTH )
    {
        char chunk[MAX_WINDOW_INFORMATION_CHUNK + 1];

        if( strlen( pBodyEntry ) < MAX_WINDOW_INFORMATION_CHUNK )
        {
            strcpy( chunk, pBodyEntry );
        }
        else
        {
            strncpy( chunk, pBodyEntry, MAX_WINDOW_INFORMATION_CHUNK );
            chunk[MAX_WINDOW_INFORMATION_CHUNK] = 0; // strncpy doesn't always append the null terminator
        }

        char_count += strlen( chunk );

        pBodyEntry = pBodyEntry + char_count;

        const bool moreToCome = pBodyEntry[0] != '\0' && char_count < MAX_WINDOW_INFORMATION_LENGTH;

        MESSAGE_BEGIN( MSG_ONE, gmsgInformationWindow, nullptr, player );
            WRITE_BYTE( page );
            if( moreToCome ) {
            WRITE_BYTE( window_sending::Appending );
            }
            else {
            WRITE_BYTE( window_sending::Finish );
            } 
            WRITE_STRING( title );
        MESSAGE_END();
    }
}
#endif
