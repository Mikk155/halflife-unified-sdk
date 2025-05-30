//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Functionality for the observer chase camera
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "pm_shared.h"
#include "UserMessages.h"

void CBasePlayer::Observer_FindNextPlayer( bool bReverse )
{
    // MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
    //                only a subset of the players. e.g. Make it check the target's team.

    int iStart;
    if( m_hObserverTarget )
        iStart = m_hObserverTarget->entindex();
    else
        iStart = entindex();
    int iCurrent = iStart;
    m_hObserverTarget = nullptr;
    int iDir = bReverse ? -1 : 1;

    do
    {
        iCurrent += iDir;

        // Loop through the clients
        if( iCurrent > gpGlobals->maxClients )
            iCurrent = 1;
        if( iCurrent < 1 )
            iCurrent = gpGlobals->maxClients;

        CBasePlayer* pEnt = UTIL_PlayerByIndex( iCurrent );
        if( !pEnt )
            continue;
        if( pEnt == this )
            continue;
        // Don't spec observers or players who haven't picked a class yet
        if( pEnt->IsObserver() || ( pEnt->pev->effects & EF_NODRAW ) != 0 )
            continue;

        // MOD AUTHORS: Add checks on target here.

        m_hObserverTarget = pEnt;
        break;

    } while( iCurrent != iStart );

    // Did we find a target?
    if( m_hObserverTarget )
    {
        // Move to the target
        SetOrigin( m_hObserverTarget->pev->origin );

        // Logger->debug("Now Tracking {}", STRING(m_hObserverTarget->pev->netname));

        // Store the target in pev so the physics DLL can get to it
        if( pev->iuser1 != OBS_ROAMING )
            pev->iuser2 = m_hObserverTarget->entindex();
    }
}

void CBasePlayer::Observer_HandleButtons()
{
    // Slow down mouse clicks
    if( m_flNextObserverInput > gpGlobals->time )
        return;

    // Jump changes from modes: Chase to Roaming
    if( ( m_afButtonPressed & IN_JUMP ) != 0 )
    {
        if( pev->iuser1 == OBS_CHASE_LOCKED )
            Observer_SetMode( OBS_CHASE_FREE );

        else if( pev->iuser1 == OBS_CHASE_FREE )
            Observer_SetMode( OBS_IN_EYE );

        else if( pev->iuser1 == OBS_IN_EYE )
            Observer_SetMode( OBS_ROAMING );

        else if( pev->iuser1 == OBS_ROAMING )
            Observer_SetMode( OBS_MAP_FREE );

        else if( pev->iuser1 == OBS_MAP_FREE )
            Observer_SetMode( OBS_MAP_CHASE );

        else
            Observer_SetMode( OBS_CHASE_FREE ); // don't use OBS_CHASE_LOCKED anymore

        m_flNextObserverInput = gpGlobals->time + 0.2;
    }

    // Attack moves to the next player
    if( ( m_afButtonPressed & IN_ATTACK ) != 0 ) //&& pev->iuser1 != OBS_ROAMING )
    {
        Observer_FindNextPlayer( false );

        m_flNextObserverInput = gpGlobals->time + 0.2;
    }

    // Attack2 moves to the prev player
    if( ( m_afButtonPressed & IN_ATTACK2 ) != 0 ) // && pev->iuser1 != OBS_ROAMING )
    {
        Observer_FindNextPlayer( true );

        m_flNextObserverInput = gpGlobals->time + 0.2;
    }
}

void CBasePlayer::Observer_CheckTarget()
{
    if( pev->iuser1 == OBS_ROAMING )
        return;

    // try to find a traget if we have no current one
    if( m_hObserverTarget == nullptr )
    {
        Observer_FindNextPlayer( false );

        if( m_hObserverTarget == nullptr )
        {
            // no target found at all

            int lastMode = pev->iuser1;

            Observer_SetMode( OBS_ROAMING );

            m_iObserverLastMode = lastMode; // don't overwrite users lastmode

            return; // we still have np target return
        }
    }

    CBasePlayer* target = UTIL_PlayerByIndex( m_hObserverTarget->entindex() );

    if( !target )
    {
        Observer_FindNextPlayer( false );
        return;
    }

    // check taget
    if( target->pev->deadflag == DEAD_DEAD )
    {
        if( ( target->m_fDeadTime + 2.0f ) < gpGlobals->time )
        {
            // 3 secs after death change target
            Observer_FindNextPlayer( false );
            return;
        }
    }
}

void CBasePlayer::Observer_CheckProperties()
{
    // try to find a traget if we have no current one
    if( pev->iuser1 == OBS_IN_EYE && m_hObserverTarget != nullptr )
    {
        CBasePlayer* target = UTIL_PlayerByIndex( m_hObserverTarget->entindex() );

        if( !target )
            return;

        int weapon = ( target->m_pActiveWeapon != nullptr ) ? target->m_pActiveWeapon->m_iId : 0;
        // use fov of tracked client
        if( m_iFOV != target->m_iFOV || m_iObserverWeapon != weapon )
        {
            m_iFOV = target->m_iFOV;
            m_iClientFOV = m_iFOV;
            // write fov before wepon data, so zoomed crosshair is set correctly
            MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, nullptr, this );
            WRITE_BYTE( m_iFOV );
            MESSAGE_END();


            m_iObserverWeapon = weapon;
            // send weapon update
            MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, nullptr, this );
            WRITE_BYTE( 1 ); // 1 = current weapon, not on target
            WRITE_BYTE( m_iObserverWeapon );
            WRITE_BYTE( 0 ); // clip
            MESSAGE_END();
        }
    }
    else
    {
        m_iFOV = 90;

        if( m_iObserverWeapon != 0 )
        {
            m_iObserverWeapon = 0;

            MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, nullptr, this );
            WRITE_BYTE( 1 ); // 1 = current weapon
            WRITE_BYTE( m_iObserverWeapon );
            WRITE_BYTE( 0 ); // clip
            MESSAGE_END();
        }
    }
}

void CBasePlayer::Observer_SetMode( int iMode )
{

    // Just abort if we're changing to the mode we're already in
    if( iMode == pev->iuser1 )
        return;

    // is valid mode ?
    if( iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE )
        iMode = OBS_IN_EYE; // now it is
    // verify observer target again
    if( m_hObserverTarget != nullptr )
    {
        CBasePlayer* pEnt = ToBasePlayer( m_hObserverTarget );

        if( pEnt == this || pEnt == nullptr )
            m_hObserverTarget = nullptr;
        else if( pEnt->IsObserver() || ( pEnt->pev->effects & EF_NODRAW ) != 0 )
            m_hObserverTarget = nullptr;
    }

    // set spectator mode
    pev->iuser1 = iMode;

    // if we are not roaming, we need a valid target to track
    if( ( iMode != OBS_ROAMING ) && ( m_hObserverTarget == nullptr ) )
    {
        Observer_FindNextPlayer( false );

        // if we didn't find a valid target switch to roaming
        if( m_hObserverTarget == nullptr )
        {
            ClientPrint( this, HUD_PRINTCENTER, "#Spec_NoTarget" );
            pev->iuser1 = OBS_ROAMING;
        }
    }

    // set target if not roaming
    if( pev->iuser1 == OBS_ROAMING )
    {
        pev->iuser2 = 0;
    }
    else
        pev->iuser2 = m_hObserverTarget->entindex();

    pev->iuser3 = 0; // clear second target from death cam

    // print spepctaor mode on client screen

    char modemsg[16];
    sprintf( modemsg, "#Spec_Mode%i", pev->iuser1 );
    ClientPrint( this, HUD_PRINTCENTER, modemsg );

    m_iObserverLastMode = iMode;
}
