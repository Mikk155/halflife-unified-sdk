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

#include "DataMap.h"

struct globalvars_t
{
    float time;
    float frametime;
    float force_retouch;
    string_t mapname;
    string_t startspot;
    [[deprecated("Use g_GameMode->IsGamemode( \"deathmatch\"sv ) instead")]] float deathmatch;
    [[deprecated("Use g_GameMode->IsGamemode( \"coop\"sv ) instead")]] float coop;
    [[deprecated("Use g_GameMode->IsTeamPlay() instead")]] float teamplay;
    float serverflags;
    float found_secrets;
    Vector v_forward;
    Vector v_up;
    Vector v_right;
    float trace_allsolid;
    float trace_startsolid;
    float trace_fraction;
    Vector trace_endpos;
    Vector trace_plane_normal;
    float trace_plane_dist;
    edict_t* trace_ent;
    float trace_inopen;
    float trace_inwater;
    int trace_hitgroup;
    int trace_flags;
    int msg_entity;
    [[deprecated("Use the music client command instead")]] int cdAudioTrack;
    int maxClients;
    int maxEntities;
    const char* pStringBase;

    void* pSaveData;
    Vector vecLandmarkOffset;
};

constexpr int NUM_ENT_CONTROLLERS = 4;
constexpr int NUM_ENT_BLENDERS = 2;

struct entvars_t
{
    DECLARE_CLASS_NOBASE(entvars_t);
    DECLARE_SIMPLE_DATAMAP();

    string_t classname;
    string_t globalname;

    Vector origin;
    Vector oldorigin;
    Vector velocity;
    Vector basevelocity;
    Vector clbasevelocity; // Base velocity that was passed in to server physics so
                           //  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
    Vector movedir;

    Vector angles;     // Model angles
    Vector avelocity;  // angle velocity (degrees per second)
    Vector punchangle; // auto-decaying view angle adjustment
    Vector v_angle;    // Viewing angle (player only)

    // For parametric entities
    Vector endpos;
    Vector startpos;
    float impacttime;
    float starttime;

    int fixangle; // 0:nothing, 1:force view angles, 2:add avelocity
    float idealpitch;
    float pitch_speed;
    float ideal_yaw;
    float yaw_speed;

    int modelindex;
    string_t model;

    string_t viewmodel;   // player's viewmodel
    string_t weaponmodel; // what other players see

    Vector absmin; // BB max translated to world coord
    Vector absmax; // BB max translated to world coord
    Vector mins;   // local BB min
    Vector maxs;   // local BB max
    Vector size;   // maxs - mins

    float ltime;
    float nextthink;

    int movetype;
    int solid;

    int skin;
    int body; // sub-model selection for studiomodels
    int effects;

    float gravity;  // % of "normal" gravity
    float friction; // inverse elasticity of MOVETYPE_BOUNCE

    int light_level;

    int sequence;                         // animation sequence
    int gaitsequence;                     // movement animation sequence for player (0 for none)
    float frame;                          // % playback position in animation sequences (0..255)
    float animtime;                       // world time when frame was set
    float framerate;                      // animation playback rate (-8x to 8x)
    byte controller[NUM_ENT_CONTROLLERS]; // bone controller setting (0..255)
    byte blending[NUM_ENT_BLENDERS];      // blending amount between sub-sequences (0..255)

    float scale; // sprite rendering scale (0..255)

    int rendermode;
    float renderamt;
    Vector rendercolor;
    int renderfx;

    float health;
    float frags;
    int weapons; // bit mask for available weapons
    float takedamage;

    int deadflag;
    Vector view_ofs; // eye position

    int button;
    int impulse;

    edict_t* chain; // Entity pointer when linked into a linked list
    edict_t* dmg_inflictor;
    edict_t* enemy;  //!< Used by some NPC movement code in the engine to determine movement destination
    edict_t* aiment; // entity pointer when MOVETYPE_FOLLOW
    edict_t* owner;
    edict_t* groundentity;

    int spawnflags;
    int flags;

    int colormap; // lowbyte topcolor, highbyte bottomcolor
    string_t team;

    float max_health;
    float teleport_time;
    float armortype;
    float armorvalue;
    WaterLevel waterlevel;
    int watertype;

    string_t target;
    string_t targetname;
    string_t netname;
    string_t message;

    float dmg_take;
    float dmg_save;
    float dmg;
    float dmgtime;

    string_t noise;
    string_t noise1;
    string_t noise2;
    string_t noise3;

    float speed;
    float air_finished;
    float pain_finished;
    float radsuit_finished;

    edict_t* pContainingEntity;

    int playerclass;
    float maxspeed;

    float fov;
    int weaponanim;

    int pushmsec;

    int bInDuck;
    int flTimeStepSound;
    int flSwimTime;
    int flDuckTime;
    int iStepLeft;
    float flFallVelocity;

    int gamestate;

    int oldbuttons;

    int groupinfo;

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
    edict_t* euser1;
    edict_t* euser2;
    edict_t* euser3;
    edict_t* euser4;
};
