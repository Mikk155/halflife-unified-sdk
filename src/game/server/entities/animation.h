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

#define ACTIVITY_NOT_AVAILABLE -1

#include "monsterevent.h"

class CBaseEntity;

bool IsSoundEvent( int eventNumber );

int LookupActivity( void* pmodel, entvars_t* pev, int activity );
int LookupActivityHeaviest( void* pmodel, entvars_t* pev, int activity );
int LookupSequence( void* pmodel, const char* label );
void GetSequenceInfo( void* pmodel, entvars_t* pev, float& flFrameRate, float& flGroundSpeed );
int GetSequenceFlags( void* pmodel, entvars_t* pev );
float SetController( void* pmodel, entvars_t* pev, int iController, float flValue );
float SetBlending( void* pmodel, entvars_t* pev, int iBlender, float flValue );
void GetEyePosition( void* pmodel, Vector& vecEyePosition );
void SequencePrecache( CBaseEntity* self, void* pmodel, const char* pSequenceName );
int FindTransition( void* pmodel, int iEndingAnim, int iGoalAnim, int* piDir );
void SetBodygroup( void* pmodel, entvars_t* pev, int iGroup, int iValue );
int GetBodygroup( void* pmodel, entvars_t* pev, int iGroup );

int GetAnimationEvent( void* pmodel, entvars_t* pev, MonsterEvent_t* pMonsterEvent, float flStart, float flEnd, int index );
bool ExtractBbox( void* pmodel, int sequence, Vector& mins, Vector& maxs );

// From /engine/studio.h
#define STUDIO_LOOPING 0x0001
