/***
 *
 *  Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *  This product contains software technology licensed from Id
 *  Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *  All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

struct edict_t;
struct physent_t;
struct pmtrace_t;

#define EVENT_API_VERSION 1

struct event_api_t
{
    int version;
    /*[[deprecated("Use EV_PlaySound in ISoundSystem.h instead")]]*/ void (*EV_PlaySound)(
        int ent, const float* origin, int channel, const char* sample, float volume, float attenuation, int fFlags, int pitch);
    [[deprecated("Use EV_StopSound in ISoundSystem.h instead")]] void (*EV_StopSound)(int ent, int channel, const char* sample);
    int (*EV_FindModelIndex)(const char* pmodel);
    int (*EV_IsLocal)(int playernum);
    int (*EV_LocalPlayerDucking)();
    void (*EV_LocalPlayerViewheight)(float*);
    void (*EV_LocalPlayerBounds)(int hull, float* mins, float* maxs);
    int (*EV_IndexFromTrace)(pmtrace_t* pTrace);
    physent_t* (*EV_GetPhysent)(int idx);
    void (*EV_SetUpPlayerPrediction)(int dopred, int bIncludeLocalClient);
    void (*EV_PushPMStates)();
    void (*EV_PopPMStates)();
    void (*EV_SetSolidPlayers)(int playernum);
    void (*EV_SetTraceHull)(int hull);
    void (*EV_PlayerTrace)(const float* start, const float* end, int traceFlags, int ignore_pe, pmtrace_t* tr);
    void (*EV_WeaponAnimation)(int sequence, int body);
    unsigned short (*EV_PrecacheEvent)(int type, const char* psz);
    void (*EV_PlaybackEvent)(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, const float* origin, const float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
    const char* (*EV_TraceTexture)(int ground, const float* vstart, const float* vend);
    void (*EV_StopAllSounds)(int entnum, int entchannel);
    void (*EV_KillEvents)(int entnum, const char* eventname);
};
