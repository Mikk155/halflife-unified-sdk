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

struct pmplane_t
{
    Vector normal;
    float dist;
};

struct pmtrace_t
{
    qboolean allsolid;        // if true, plane is not valid
    qboolean startsolid;      // if true, the initial point was in a solid area
    qboolean inopen, inwater; // End point is in empty space or in water
    float fraction;           // time completed, 1.0 = didn't hit anything
    Vector endpos;            // final position
    pmplane_t plane;          // surface normal at impact
    int ent;                  // entity at impact
    Vector deltavelocity;     // Change in player's velocity caused by impact.
                              // Only run on server.
    int hitgroup;
};
