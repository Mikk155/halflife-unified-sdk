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

// Enumeration used for "Light type" so we get the correct client cvar to display or not.
enum DynamicLightType
{
    Muzzleflash = 1,
    StudioModelRendering,
    Explosions
};

struct dlight_t
{
    Vector origin;
    float radius;
    color24 color;
    float die;      // stop lighting after this time
    float decay;    // drop this each second
    float minlight; // don't add when contributing less
    int key;
    qboolean dark; // subtracts light instead of adding
};
