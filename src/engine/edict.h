//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

#define MAX_ENT_LEAFS 48

#include "progdefs.h"

struct edict_t
{
    qboolean free;
    int serialnumber;
    link_t area; // linked to a division node or leaf

    int headnode; // -1 to use normal leaf check
    int num_leafs;
    short leafnums[MAX_ENT_LEAFS];

    float freetime; // sv.time when the object was freed

    void* pvPrivateData; // Alloced and freed by engine, used by DLLs

    entvars_t v; // C exported fields from progs

    // other fields from progs come immediately after
};
