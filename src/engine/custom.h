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
// Customization.h

#pragma once

#include "const.h"

/////////////////
// Customization
// passed to pfnPlayerCustomization
// For automatic downloading.
enum resourcetype_t
{
    t_sound = 0,
    t_skin,
    t_model,
    t_decal,
    t_generic,
    t_eventscript,
    t_world, // Fake type for world, is really t_model
};

#define RES_FATALIFMISSING (1 << 0) // Disconnect if we can't get this file.
#define RES_WASMISSING (1 << 1)     // Do we have the file locally, did we get it ok?
#define RES_CUSTOM (1 << 2)         // Is this resource one that corresponds to another player's customization
// or is it a server startup resource.
#define RES_REQUESTED (1 << 3) // Already requested a download of this one
#define RES_PRECACHED (1 << 4) // Already precached
#define RES_ALWAYS (1 << 5)    // download always even if available on client
#define RES_CHECKFILE (1 << 7) // check file on client

struct resource_t
{
    char szFileName[MAX_QPATH]; // File name to download/precache.
    resourcetype_t type;        // t_sound, t_skin, t_model, t_decal.
    int nIndex;                 // For t_decals
    int nDownloadSize;          // Size in Bytes if this must be downloaded.
    unsigned char ucFlags;

    // For handling client to client resource propagation
    unsigned char rgucMD5_hash[16]; // To determine if we already have it.
    unsigned char playernum;        // Which player index this resource is associated with, if it's a custom resource.

    unsigned char rguc_reserved[32]; // For future expansion
    resource_t* pNext;               // Next in chain.
    resource_t* pPrev;
};

struct customization_t
{
    qboolean bInUse;        // Is this customization in use;
    resource_t resource;    // The resource_t for this customization
    qboolean bTranslated;   // Has the raw data been translated into a useable format?
                            //  (e.g., raw decal .wad make into texture_t *)
    int nUserData1;         // Customization specific data
    int nUserData2;         // Customization specific data
    void* pInfo;            // Buffer that holds the data structure that references the data (e.g., the cachewad_t)
    void* pBuffer;          // Buffer that holds the data for the customization (the raw .wad data)
    customization_t* pNext; // Next in chain
};

#define FCUST_FROMHPAK (1 << 0)
#define FCUST_WIPEDATA (1 << 1)
#define FCUST_IGNOREINIT (1 << 2)
