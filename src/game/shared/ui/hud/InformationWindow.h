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

#pragma once

#include "cbase.h"
#include "utils/GameSystem.h"

#define MAX_PAGES 16

enum window_sending
{
    // Cleanup this page
    Cleanup = 0,
    // Setup the page's title
    Title,
    // Append text to the body
    Appending,
    // Finish appending to body. open motd
    Finish,
    // Just open an already sent page
    JustOpen
};

#ifdef CLIENT_DLL
struct window_data
{
    bool active = false;
    std::string title;
    std::string body;
};
#endif

class CInformationWindow final : public IGameSystem
{
public:
    std::shared_ptr<spdlog::logger> m_Logger; // I'll remove this after we get the system working.

    const char* GetName() const override { return "InformationWindow"; }

    bool Initialize() override;

    void PostInitialize() override {}

    void Shutdown() override {}

    const int ClampPages( int pages );

#ifndef CLIENT_DLL
    const char* GetBodyString( const char* str );
    void SendInformation( CBasePlayer* player, int page, const char* title, const char* body );
    void ClearInformation( CBasePlayer* player, int page );
#endif

#ifdef CLIENT_DLL
    void OpenWindow( int page );

private:
    void MsgFunc_WindowInformation( BufferReader& reader );

    std::vector<window_data> m_window_pages;
#endif
};

inline CInformationWindow g_InformationWindow;
