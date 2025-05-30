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
// pm_defs.h

#pragma once

#include "Platform.h"
#define MAX_PHYSENTS 600 // Must have room for all entities in the world.
#define MAX_MOVEENTS 64
#define MAX_CLIP_PLANES 5

#define PM_NORMAL 0x00000000
#define PM_STUDIO_IGNORE 0x00000001 // Skip studio models
#define PM_STUDIO_BOX 0x00000002    // Use boxes for non-complex studio models (even in traceline)
#define PM_GLASS_IGNORE 0x00000004    // Ignore entities with non-normal rendermode
#define PM_WORLD_ONLY 0x00000008    // Only trace against the world

// Values for flags parameter of PM_TraceLine
#define PM_TRACELINE_PHYSENTSONLY 0
#define PM_TRACELINE_ANYVISIBLE 1


#include "pm_info.h"

// PM_PlayerTrace results.
#include "pmtrace.h"

#include "usercmd.h"

struct hull_t;
struct model_t;
struct movevars_t;

// physent_t
struct physent_t
{
    char name[32]; // Name of model, or "player" or "world".
    int player;
    Vector origin;          // Model's origin in world coordinates.
    model_t* model;          // only for bsp models
    model_t* studiomodel; // SOLID_BBOX, but studio clip intersections.
    Vector mins, maxs;      // only for non-bsp models
    int info;              // For client or server to use to identify (index into edicts or cl_entities)
    Vector angles;          // rotated entities need this info for hull testing to work.

    int solid;        // Triggers and func_door type WATER brushes are SOLID_NOT
    int skin;        // BSP Contents for such things like fun_door water brushes.
    int rendermode; // So we can ignore glass

    // Complex collision detection.
    float frame;
    int sequence;
    byte controller[4];
    byte blending[2];

    int movetype;
    int takedamage;
    int blooddecal;
    int team;
    int classnumber;

    int iuser1; // ON USE! for CBasePlayer
    int iuser2; // ON USE! for CBasePlayer
    int iuser3; // ON USE! for CBasePlayer
    int iuser4; // ON USE! for CBasePlayer
    float fuser1; // ON USE! for CBasePlayer
    float fuser2; // ON USE! for CBasePlayer
    float fuser3; // ON USE! for CBasePlayer
    float fuser4;
    Vector vuser1; // ON USE! for CBasePlayer
    Vector vuser2; // ON USE! for CBasePlayer
    Vector vuser3; // ON USE! for CBasePlayer
    Vector vuser4; // ON USE! for CBasePlayer
};


struct playermove_t
{
    int player_index; // So we don't try to run the PM_CheckStuck nudging too quickly.
    qboolean server;  // For debugging, are we running physics code on server side?

    qboolean multiplayer; // 1 == multiplayer server
    float time;              // realtime on host, for reckoning duck timing
    float frametime;      // Duration of this frame

    Vector forward, right, up; // Vectors for angles
    // player state
    Vector origin;         // Movement origin.
    Vector angles;         // Movement view angles.
    Vector oldangles;     // Angles before movement view angles were looked at.
    Vector velocity;     // Current movement direction.
    Vector movedir;         // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
    Vector basevelocity; // Velocity of the conveyor we are standing, e.g.

    // For ducking/dead
    Vector view_ofs;  // Our eye position.
    float flDuckTime; // Time we started duck
    qboolean bInDuck; // In process of ducking or ducked already?

    // For walking/falling
    int flTimeStepSound; // Next time we can play a step sound
    int iStepLeft;

    float flFallVelocity;
    Vector punchangle;

    float flSwimTime;

    float flNextPrimaryAttack;

    int effects; // MUZZLE FLASH, e.g.

    int flags;       // FL_ONGROUND, FL_DUCKING, etc.
    int usehull;   // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
    float gravity; // Our current gravity and friction.
    float friction;
    int oldbuttons;         // Buttons last usercmd
    float waterjumptime; // Amount of time left in jumping out of water cycle.
    qboolean dead;         // Are we a dead player?
    int deadflag;
    int spectator; // Should we use spectator physics model?
    int movetype;  // Our movement type, NOCLIP, WALK, FLY

    int onground;
    int waterlevel;
    int watertype;
    int oldwaterlevel;

    char sztexturename[256];
    char chtexturetype;

    float maxspeed;
    float clientmaxspeed; // Player specific maxspeed

    int iuser1; // ON USE! for CBasePlayer
    int iuser2; // ON USE! for CBasePlayer
    int iuser3; // ON USE! for CBasePlayer
    int iuser4; // ON USE! for CBasePlayer
    float fuser1; // ON USE! for CBasePlayer
    float fuser2; // ON USE! for CBasePlayer
    float fuser3; // ON USE! for CBasePlayer
    float fuser4;
    Vector vuser1; // ON USE! for CBasePlayer
    Vector vuser2; // ON USE! for CBasePlayer
    Vector vuser3; // ON USE! for CBasePlayer
    Vector vuser4; // ON USE! for CBasePlayer
    // world state
    // Number of entities to clip against.
    int numphysent;
    physent_t physents[MAX_PHYSENTS];
    // Number of momvement entities (ladders)
    int nummoveent;
    // just a list of ladders
    physent_t moveents[MAX_MOVEENTS];

    // All things being rendered, for tracing against things you don't actually collide with
    int numvisent;
    physent_t visents[MAX_PHYSENTS];

    // input to run through physics.
    usercmd_t cmd;

    // Trace results for objects we collided with.
    int numtouch;
    pmtrace_t touchindex[MAX_PHYSENTS];

    char physinfo[MAX_PHYSINFO_STRING]; // Physics info string

    movevars_t* movevars;
    Vector player_mins[4];
    Vector player_maxs[4];

    // Common functions
    const char* ( *PM_Info_ValueForKey )( const char* s, const char* key );
    void ( *PM_Particle )( float* origin, int color, float life, int zpos, int zvel );
    int ( *PM_TestPlayerPosition )( float* pos, pmtrace_t* ptrace );
    void ( *Con_NPrintf )( int idx, const char* fmt, ... );
    void ( *Con_DPrintf )( const char* fmt, ... );
    void ( *Con_Printf )( const char* fmt, ... );
    double ( *Sys_FloatTime )();
    void ( *PM_StuckTouch )( int hitent, pmtrace_t* ptraceresult );
    int (*PM_PointContents)(float* p, int* truecontents /*filled in if this is non-null*/);
    int ( *PM_TruePointContents )( float* p );
    int ( *PM_HullPointContents )( hull_t* hull, int num, float* p );
    pmtrace_t ( *PM_PlayerTrace )( float* start, float* end, int traceFlags, int ignore_pe );
    pmtrace_t* ( *PM_TraceLine )( float* start, float* end, int flags, int usehulll, int ignore_pe );
    int32 ( *RandomLong )( int32 lLow, int32 lHigh );
    float ( *RandomFloat )( float flLow, float flHigh );
    int ( *PM_GetModelType )( model_t* mod );
    void ( *PM_GetModelBounds )( model_t* mod, float* mins, float* maxs );
    void* ( *PM_HullForBsp )( physent_t* pe, float* offset );
    float ( *PM_TraceModel )( physent_t* pEnt, const float* start, const float* end, trace_t* trace );
    int ( *COM_FileSize )( const char* filename );
    byte* ( *COM_LoadFile )( const char* path, int usehunk, int* pLength );
    void ( *COM_FreeFile )( void* buffer );
    char* ( *memfgets )( byte* pMemFile, int fileSize, int* pFilePos, char* pBuffer, int bufferSize );

    // Functions
    // Run functions for this frame?
    qboolean runfuncs;
    void ( *PM_PlaySound )( int channel, const char* sample, float volume, float attenuation, int fFlags, int pitch );
    const char* ( *PM_TraceTexture )( int ground, float* vstart, float* vend );
    void ( *PM_PlaybackEventFull )( int flags, int clientindex, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

    pmtrace_t ( *PM_PlayerTraceEx )( float* start, float* end, int traceFlags, int ( *pfnIgnore )( physent_t* pe ) );
    int ( *PM_TestPlayerPositionEx )( float* pos, pmtrace_t* ptrace, int ( *pfnIgnore )( physent_t* pe ) );
    pmtrace_t* ( *PM_TraceLineEx )( float* start, float* end, int flags, int usehulll, int ( *pfnIgnore )( physent_t* pe ) );
};
