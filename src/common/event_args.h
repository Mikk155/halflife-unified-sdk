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

// Event was invoked with stated origin
#define FEVENT_ORIGIN (1 << 0)

// Event was invoked with stated angles
#define FEVENT_ANGLES (1 << 1)

struct event_args_t
{
    int flags;

    // Transmitted
    int entindex;

    Vector origin;
    Vector angles;
    Vector velocity;

    int ducking;

    float fparam1;
    float fparam2;

    int iparam1;
    int iparam2;

    int bparam1;
    int bparam2;
};
