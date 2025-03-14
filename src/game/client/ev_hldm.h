//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

void EV_HLDM_GunshotDecalTrace( pmtrace_t* pTrace, char* decalName );
void EV_HLDM_DecalGunshot( pmtrace_t* pTrace, int iBulletType );
void EV_HLDM_CheckTracer( int idx, const Vector& vecSrc, const Vector& end, const Vector& forward, const Vector& right,
    int iBulletType, int iTracerFreq, int* tracerCount );
void EV_HLDM_FireBullets( int idx,
    const Vector& forward, const Vector& right, const Vector& up,
    int cShots, const Vector& vecSrc, const Vector& vecDirShooting,
    float flDistance, int iBulletType, int iTracerFreq, int* tracerCount, float flSpreadX, float flSpreadY );

// HLDM
void EV_FireGlock1( event_args_t* args );
void EV_FireGlock2( event_args_t* args );
void EV_FireShotGunSingle( event_args_t* args );
void EV_FireShotGunDouble( event_args_t* args );
void EV_FireMP5( event_args_t* args );
void EV_FireMP52( event_args_t* args );
void EV_FirePython( event_args_t* args );
void EV_FireGauss( event_args_t* args );
void EV_SpinGauss( event_args_t* args );
void EV_Crowbar( event_args_t* args );
void EV_FireCrossbow( event_args_t* args );
void EV_FireCrossbow2( event_args_t* args );
void EV_FireRpg( event_args_t* args );
void EV_EgonFire( event_args_t* args );
void EV_EgonStop( event_args_t* args );
void EV_HornetGunFire( event_args_t* args );
void EV_TripmineFire( event_args_t* args );
void EV_SnarkFire( event_args_t* args );

void EV_FireEagle( event_args_t* args );
void EV_Pipewrench( event_args_t* args );
void EV_FireM249( event_args_t* args );
void EV_FireDisplacer( event_args_t* args );
void EV_FireShockRifle( event_args_t* args );
void EV_FireSpore( event_args_t* args );
void EV_SniperRifle( event_args_t* args );
void EV_Knife( event_args_t* args );
void EV_PenguinFire( event_args_t* args );

void SetLocalBody( int id, int body );
