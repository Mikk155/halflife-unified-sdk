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

#include "cbase.h"
#include "animation.h"
#include "studio.h"

BEGIN_DATAMAP( CBaseAnimating )
    DEFINE_FIELD( m_flFrameRate, FIELD_FLOAT ),
    DEFINE_FIELD( m_flGroundSpeed, FIELD_FLOAT ),
    DEFINE_FIELD( m_flLastEventCheck, FIELD_TIME ),
    DEFINE_FIELD( m_fSequenceFinished, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_fSequenceLoops, FIELD_BOOLEAN ),
END_DATAMAP();

float CBaseAnimating::StudioFrameAdvance( float flInterval )
{
    if( flInterval == 0.0 )
    {
        flInterval = ( gpGlobals->time - pev->animtime );
        if( flInterval <= 0.001 )
        {
            pev->animtime = gpGlobals->time;
            return 0.0;
        }
    }
    if( 0 == pev->animtime )
        flInterval = 0.0;

    pev->frame += flInterval * m_flFrameRate * pev->framerate;
    pev->animtime = gpGlobals->time;

    if( pev->frame < 0.0 || pev->frame >= 256.0 )
    {
        if( m_fSequenceLoops )
            pev->frame -= (int)( pev->frame / 256.0 ) * 256.0;
        else
            pev->frame = ( pev->frame < 0.0 ) ? 0 : 255;
        m_fSequenceFinished = true; // just in case it wasn't caught in GetEvents
    }

    return flInterval;
}

int CBaseAnimating::LookupActivity( int activity )
{
    ASSERT( activity != 0 );
    void* pmodel = GET_MODEL_PTR( edict() );

    return ::LookupActivity( pmodel, pev, activity );
}

int CBaseAnimating::LookupActivityHeaviest( int activity )
{
    void* pmodel = GET_MODEL_PTR( edict() );

    return ::LookupActivityHeaviest( pmodel, pev, activity );
}

int CBaseAnimating::LookupSequence( const char* label )
{
    void* pmodel = GET_MODEL_PTR( edict() );

    return ::LookupSequence( pmodel, label );
}

void CBaseAnimating::ResetSequenceInfo()
{
    void* pmodel = GET_MODEL_PTR( edict() );

    GetSequenceInfo( pmodel, pev, m_flFrameRate, m_flGroundSpeed );
    m_fSequenceLoops = ( ( GetSequenceFlags() & STUDIO_LOOPING ) != 0 );
    pev->animtime = gpGlobals->time;
    pev->framerate = 1.0;
    m_fSequenceFinished = false;
    m_flLastEventCheck = gpGlobals->time;
}

int CBaseAnimating::GetSequenceFlags()
{
    void* pmodel = GET_MODEL_PTR( edict() );

    return ::GetSequenceFlags( pmodel, pev );
}

void CBaseAnimating::DispatchAnimEvents( float flInterval )
{
    MonsterEvent_t event;

    void* pmodel = GET_MODEL_PTR( edict() );

    if( !pmodel )
    {
        CBaseEntity::Logger->debug( "Gibbed monster is thinking!" );
        return;
    }

    // FIXME: I have to do this or some events get missed, and this is probably causing the problem below
    flInterval = 0.1;

    // FIX: this still sometimes hits events twice
    float flStart = pev->frame + ( m_flLastEventCheck - pev->animtime ) * m_flFrameRate * pev->framerate;
    float flEnd = pev->frame + flInterval * m_flFrameRate * pev->framerate;
    m_flLastEventCheck = pev->animtime + flInterval;

    m_fSequenceFinished = false;
    if( flEnd >= 256 || flEnd <= 0.0 )
        m_fSequenceFinished = true;

    int index = 0;

    while( ( index = GetAnimationEvent( pmodel, pev, &event, flStart, flEnd, index ) ) != 0 )
    {
        HandleAnimEvent( &event );
    }
}

float CBaseAnimating::SetBoneController( int iController, float flValue )
{
    void* pmodel = GET_MODEL_PTR( edict() );

    return SetController( pmodel, pev, iController, flValue );
}

void CBaseAnimating::InitBoneControllers()
{
    void* pmodel = GET_MODEL_PTR( edict() );

    SetController( pmodel, pev, 0, 0.0 );
    SetController( pmodel, pev, 1, 0.0 );
    SetController( pmodel, pev, 2, 0.0 );
    SetController( pmodel, pev, 3, 0.0 );
}

float CBaseAnimating::SetBlending( int iBlender, float flValue )
{
    void* pmodel = GET_MODEL_PTR( edict() );

    return ::SetBlending( pmodel, pev, iBlender, flValue );
}

void CBaseAnimating::GetBonePosition( int iBone, Vector& origin, Vector& angles )
{
    GET_BONE_POSITION( edict(), iBone, origin, angles );
}

void CBaseAnimating::GetAttachment( int iAttachment, Vector& origin, Vector& angles )
{
    GET_ATTACHMENT( edict(), iAttachment, origin, angles );
}

int CBaseAnimating::FindTransition( int iEndingSequence, int iGoalSequence, int* piDir )
{
    void* pmodel = GET_MODEL_PTR( edict() );

    if( piDir == nullptr )
    {
        int iDir;
        int sequence = ::FindTransition( pmodel, iEndingSequence, iGoalSequence, &iDir );
        if( iDir != 1 )
            return -1;
        else
            return sequence;
    }

    return ::FindTransition( pmodel, iEndingSequence, iGoalSequence, piDir );
}

void CBaseAnimating::GetAutomovement( Vector& origin, Vector& angles, float flInterval )
{
}

void CBaseAnimating::SetBodygroup( int iGroup, int iValue )
{
    ::SetBodygroup( GET_MODEL_PTR( edict() ), pev, iGroup, iValue );
}

int CBaseAnimating::GetBodygroup( int iGroup ) const
{
    return ::GetBodygroup( GET_MODEL_PTR( edict() ), pev, iGroup );
}

int CBaseAnimating::GetBodygroupSubmodelCount( int group )
{
    auto pstudiohdr = reinterpret_cast<studiohdr_t*>( GET_MODEL_PTR( edict() ) );

    if( !pstudiohdr )
        return 1;

    if( group < 0 || group >= pstudiohdr->numbodyparts )
        return 1;

    auto pbodypart = ( mstudiobodyparts_t* )( ( byte* )pstudiohdr + pstudiohdr->bodypartindex ) + group;

    return pbodypart->nummodels;
}

bool CBaseAnimating::ExtractBbox( int sequence, Vector& mins, Vector& maxs )
{
    return ::ExtractBbox( GET_MODEL_PTR( edict() ), sequence, mins, maxs );
}

void CBaseAnimating::SetSequenceBox()
{
    Vector mins, maxs;

    // Get sequence bbox
    if( ExtractBbox( pev->sequence, mins, maxs ) )
    {
        // expand box for rotation
        // find min / max for rotations
        float yaw = pev->angles.y * ( PI / 180.0 );

        Vector xvector, yvector;
        xvector.x = cos( yaw );
        xvector.y = sin( yaw );
        yvector.x = -sin( yaw );
        yvector.y = cos( yaw );
        Vector bounds[2];

        bounds[0] = mins;
        bounds[1] = maxs;

        Vector rmin( 9999, 9999, 9999 );
        Vector rmax( -9999, -9999, -9999 );
        Vector base, transformed;

        for( int i = 0; i <= 1; i++ )
        {
            base.x = bounds[i].x;
            for( int j = 0; j <= 1; j++ )
            {
                base.y = bounds[j].y;
                for( int k = 0; k <= 1; k++ )
                {
                    base.z = bounds[k].z;

                    // transform the point
                    transformed.x = xvector.x * base.x + yvector.x * base.y;
                    transformed.y = xvector.y * base.x + yvector.y * base.y;
                    transformed.z = base.z;

                    if( transformed.x < rmin.x )
                        rmin.x = transformed.x;
                    if( transformed.x > rmax.x )
                        rmax.x = transformed.x;
                    if( transformed.y < rmin.y )
                        rmin.y = transformed.y;
                    if( transformed.y > rmax.y )
                        rmax.y = transformed.y;
                    if( transformed.z < rmin.z )
                        rmin.z = transformed.z;
                    if( transformed.z > rmax.z )
                        rmax.z = transformed.z;
                }
            }
        }
        rmin.z = 0;
        rmax.z = rmin.z + 1;
        SetSize( rmin, rmax );
    }
}
