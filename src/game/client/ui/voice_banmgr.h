//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

// This class manages the (persistent) list of squelched players.
class CVoiceBanMgr
{
public:
    CVoiceBanMgr();
    ~CVoiceBanMgr();

    // Init loads the list of squelched players from disk.
    bool Init();
    void Term();

    // Saves the state into voice_squelch.dt.
    void SaveState();

    bool GetPlayerBan( char const playerID[16] );
    void SetPlayerBan( char const playerID[16], bool bSquelch );

    // Call your callback for each banned player.
    void ForEachBannedPlayer( void ( *callback )( char id[16] ) );


protected:
    class BannedPlayer
    {
    public:
        char m_PlayerID[16];
        BannedPlayer *m_pPrev, *m_pNext;
    };

    void Clear();
    BannedPlayer* InternalFindPlayerSquelch( char const playerID[16] );
    BannedPlayer* AddBannedPlayer( char const playerID[16] );


protected:
    BannedPlayer m_PlayerHash[256];
};
