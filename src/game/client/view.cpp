// view/refresh setup functions

#include "hud.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ClientLibrary.h"
#include "ref_params.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"
#include "Exports.h"
#include "tri.h"
#include "view.h"

extern Vector vJumpOrigin;
extern Vector vJumpAngles;

void V_DropPunchAngle( float frametime, Vector& punchangle );

#include "r_studioint.h"
#include "com_model.h"
#include "kbutton.h"

extern engine_studio_api_t IEngineStudio;

extern kbutton_t in_mlook;

/*
The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.
*/

extern cvar_t* cl_forwardspeed;
extern cvar_t* chase_active;
extern cvar_t *scr_ofsx, *scr_ofsy, *scr_ofsz;
extern cvar_t* cl_vsmoothing;
extern cvar_t* cl_rollangle;
extern cvar_t* cl_rollspeed;
extern cvar_t* cl_bobtilt;

#define CAM_MODE_RELAX 1
#define CAM_MODE_FOCUS 2

Vector v_lastAngles;
float v_frametime, v_lastDistance;
float v_cameraRelaxAngle = 5.0f;
float v_cameraFocusAngle = 35.0f;
int v_cameraMode = CAM_MODE_FOCUS;
bool v_resetCamera = true;

Vector ev_punchangle;

cvar_t* scr_ofsx;
cvar_t* scr_ofsy;
cvar_t* scr_ofsz;

cvar_t* v_centermove;
cvar_t* v_centerspeed;

cvar_t* cl_bobcycle;
cvar_t* cl_bob;
cvar_t* cl_bobup;
cvar_t* cl_waterdist;
cvar_t* cl_chasedist;

// These cvars are not registered (so users can't cheat), so set the ->value field directly
// Register these cvars in V_Init() if needed for easy tweaking
cvar_t v_iyaw_cycle = {"v_iyaw_cycle", "2", 0, 2};
cvar_t v_iroll_cycle = {"v_iroll_cycle", "0.5", 0, 0.5};
cvar_t v_ipitch_cycle = {"v_ipitch_cycle", "1", 0, 1};
cvar_t v_iyaw_level = {"v_iyaw_level", "0.3", 0, 0.3};
cvar_t v_iroll_level = {"v_iroll_level", "0.1", 0, 0.1};
cvar_t v_ipitch_level = {"v_ipitch_level", "0.3", 0, 0.3};

float v_idlescale; // used by TFC for concussion grenade effect

//=============================================================================
/*
void V_NormalizeAngles( Vector& angles )
{
    int i;
    // Normalize angles
    for ( i = 0; i < 3; i++ )
    {
        if ( angles[i] > 180.0 )
        {
            angles[i] -= 360.0;
        }
        else if ( angles[i] < -180.0 )
        {
            angles[i] += 360.0;
        }
    }
}*/

/*
===================
V_InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================

void V_InterpolateAngles( float *start, float *end, float *output, float frac )
{
    int i;
    float ang1, ang2;
    float d;

    V_NormalizeAngles( start );
    V_NormalizeAngles( end );

    for ( i = 0 ; i < 3 ; i++ )
    {
        ang1 = start[i];
        ang2 = end[i];

        d = ang2 - ang1;
        if ( d > 180 )
        {
            d -= 360;
        }
        else if ( d < -180 )
        {
            d += 360;
        }

        output[i] = ang1 + d * frac;
    }

    V_NormalizeAngles( output );
} */

// Quakeworld bob code, this fixes jitters in the mutliplayer since the clock (pparams->time) isn't quite linear
float V_CalcBob( ref_params_t* pparams )
{
    static double bobtime = 0;
    static float bob = 0;
    float cycle;
    static float lasttime = 0;
    Vector vel;


    if( pparams->onground == -1 ||
        pparams->time == lasttime )
    {
        // just use old value
        return bob;
    }

    lasttime = pparams->time;

    // TODO: bobtime will eventually become a value so large that it will no longer behave properly.
    // Consider resetting the variable if a level change is detected (pparams->time < lasttime might do the trick).
    bobtime += pparams->frametime;
    cycle = bobtime - (int)( bobtime / cl_bobcycle->value ) * cl_bobcycle->value;
    cycle /= cl_bobcycle->value;

    if( cycle < cl_bobup->value )
    {
        cycle = PI * cycle / cl_bobup->value;
    }
    else
    {
        cycle = PI + PI * ( cycle - cl_bobup->value ) / ( 1.0 - cl_bobup->value );
    }

    // bob is proportional to simulated velocity in the xy plane
    // (don't count Z, or jumping messes it up)
    vel = pparams->simvel;
    vel[2] = 0;

    bob = sqrt( vel[0] * vel[0] + vel[1] * vel[1] ) * cl_bob->value;
    bob = bob * 0.3 + bob * 0.7 * sin( cycle );
    bob = std::min( bob, 4.f );
    bob = std::max( bob, -7.f );
    return bob;
}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll( Vector angles, Vector velocity, float rollangle, float rollspeed )
{
    float sign;
    float side;
    float value;
    Vector forward, right, up;

    AngleVectors( angles, forward, right, up );

    side = DotProduct( velocity, right );
    sign = side < 0 ? -1 : 1;
    side = fabs( side );

    value = rollangle;
    if( side < rollspeed )
    {
        side = side * value / rollspeed;
    }
    else
    {
        side = value;
    }
    return side * sign;
}

struct pitchdrift_t
{
    float pitchvel;
    bool nodrift;
    float driftmove;
    double laststop;
};

static pitchdrift_t pd;

void V_StartPitchDrift()
{
    if( pd.laststop == gEngfuncs.GetClientTime() )
    {
        return; // something else is keeping it from drifting
    }

    if( pd.nodrift || 0 == pd.pitchvel )
    {
        pd.pitchvel = v_centerspeed->value;
        pd.nodrift = false;
        pd.driftmove = 0;
    }
}

void V_StopPitchDrift()
{
    pd.laststop = gEngfuncs.GetClientTime();
    pd.nodrift = true;
    pd.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
void V_DriftPitch( ref_params_t* pparams )
{
    float delta, move;

    if( 0 != gEngfuncs.IsNoClipping() || 0 == pparams->onground || 0 != pparams->demoplayback || 0 != pparams->spectator )
    {
        pd.driftmove = 0;
        pd.pitchvel = 0;
        return;
    }

    // don't count small mouse motion
    if( pd.nodrift )
    {
        if( v_centermove->value > 0 && ( in_mlook.state & 1 ) == 0 )
        {
            // this is for lazy players. if they stopped, looked around and then continued
            // to move the view will be centered automatically if they move more than
            // v_centermove units.

            if( fabs( pparams->cmd->forwardmove ) < cl_forwardspeed->value )
                pd.driftmove = 0;
            else
                pd.driftmove += pparams->frametime;

            if( pd.driftmove > v_centermove->value )
            {
                V_StartPitchDrift();
            }
            else
            {
                return; // player didn't move enough
            }
        }

        return; // don't drift view
    }

    delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

    if( 0 == delta )
    {
        pd.pitchvel = 0;
        return;
    }

    move = pparams->frametime * pd.pitchvel;

    pd.pitchvel *= ( 1.0f + ( pparams->frametime * 0.25f ) ); // get faster by time

    if( delta > 0 )
    {
        if( move > delta )
        {
            pd.pitchvel = 0;
            move = delta;
        }
        pparams->cl_viewangles[PITCH] += move;
    }
    else if( delta < 0 )
    {
        if( move > -delta )
        {
            pd.pitchvel = 0;
            move = -delta;
        }
        pparams->cl_viewangles[PITCH] -= move;
    }
}

/*
==============================================================================
                        VIEW RENDERING
==============================================================================
*/

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle( ref_params_t* pparams )
{
    cl_entity_t* viewent;

    viewent = gEngfuncs.GetViewModel();
    if( !viewent )
        return;

    viewent->angles[YAW] = pparams->viewangles[YAW] + pparams->crosshairangle[YAW];
    viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
    viewent->angles[ROLL] -= v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;

    // don't apply all of the v_ipitch to prevent normally unseen parts of viewmodel from coming into view.
    viewent->angles[PITCH] -= v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * ( v_ipitch_level.value * 0.5 );
    viewent->angles[YAW] -= v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;

    viewent->curstate.angles = viewent->angles;
    viewent->latched.prevangles = viewent->angles;
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle( ref_params_t* pparams )
{
    pparams->viewangles[ROLL] += v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;
    pparams->viewangles[PITCH] += v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * v_ipitch_level.value;
    pparams->viewangles[YAW] += v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;
}


/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll( ref_params_t* pparams )
{
    float side;
    cl_entity_t* viewentity;

    viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
    if( !viewentity )
        return;

    side = V_CalcRoll( viewentity->angles, pparams->simvel, cl_rollangle->value, cl_rollspeed->value );

    pparams->viewangles[ROLL] += side;

    if( pparams->health <= 0 && ( pparams->viewheight[2] != 0 ) )
    {
        // only roll the view if the player is dead and the viewheight[2] is nonzero
        // this is so deadcam in multiplayer will work.
        pparams->viewangles[ROLL] = 80; // dead view angle
        return;
    }
}


/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef( ref_params_t* pparams )
{
    cl_entity_t *ent, *view;
    float old;

    // ent is the player model ( visible when out of body )
    ent = gEngfuncs.GetLocalPlayer();

    // view is the weapon model (only visible from inside body )
    view = gEngfuncs.GetViewModel();

    pparams->vieworg = pparams->simorg;
    pparams->viewangles = pparams->cl_viewangles;

    view->model = nullptr;

    // allways idle in intermission
    old = v_idlescale;
    v_idlescale = 1;

    V_AddIdle( pparams );

    if( 0 != gEngfuncs.IsSpectateOnly() )
    {
        // in HLTV we must go to 'intermission' position by ourself
        pparams->vieworg = gHUD.m_Spectator.m_cameraOrigin;
        pparams->viewangles = gHUD.m_Spectator.m_cameraAngles;
    }

    v_idlescale = old;

    v_cl_angles = pparams->cl_viewangles;
    v_origin = pparams->vieworg;
    v_angles = pparams->viewangles;
}

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK (ORIGIN_BACKUP - 1)

struct viewinterp_t
{
    Vector Origins[ORIGIN_BACKUP];
    float OriginTime[ORIGIN_BACKUP];

    Vector Angles[ORIGIN_BACKUP];
    float AngleTime[ORIGIN_BACKUP];

    int CurrentOrigin;
    int CurrentAngle;
};

/*
==================
V_CalcRefdef

==================
*/
void V_CalcNormalRefdef( ref_params_t* pparams )
{
    cl_entity_t *ent, *view;
    Vector angles;
    float bob, waterOffset;
    static viewinterp_t ViewInterp;

    static float oldz = 0;
    static float lasttime;

    Vector camAngles, camForward, camRight, camUp;
    cl_entity_t* pwater;

    V_DriftPitch( pparams );

    if( 0 != gEngfuncs.IsSpectateOnly() )
    {
        ent = gEngfuncs.GetEntityByIndex( g_iUser2 );
    }
    else
    {
        // ent is the player model ( visible when out of body )
        ent = gEngfuncs.GetLocalPlayer();
    }

    // view is the weapon model (only visible from inside body )
    view = gEngfuncs.GetViewModel();

    // transform the view offset by the model's matrix to get the offset from
    // model origin for the view
    bob = V_CalcBob( pparams );

    // refresh position
    pparams->vieworg = pparams->simorg;
    pparams->vieworg[2] += ( bob );
    pparams->vieworg = pparams->vieworg + pparams->viewheight;

    pparams->viewangles = pparams->cl_viewangles;

    gEngfuncs.V_CalcShake();
    gEngfuncs.V_ApplyShake( pparams->vieworg, pparams->viewangles, 1.0 );

    // never let view origin sit exactly on a node line, because a water plane can
    // dissapear when viewed with the eye exactly on it.
    // FIXME, we send origin at 1/128 now, change this?
    // the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis

    pparams->vieworg[0] += 1.0 / 32;
    pparams->vieworg[1] += 1.0 / 32;
    pparams->vieworg[2] += 1.0 / 32;

    // Check for problems around water, move the viewer artificially if necessary
    // -- this prevents drawing errors in GL due to waves

    waterOffset = 0;
    if( pparams->waterlevel >= WaterLevel::Waist )
    {
        int contents, waterDist, waterEntity;
        Vector point;
        waterDist = cl_waterdist->value;

        if( 0 != pparams->hardware )
        {
            waterEntity = gEngfuncs.PM_WaterEntity( pparams->simorg );
            if( waterEntity >= 0 && waterEntity < pparams->max_entities )
            {
                pwater = gEngfuncs.GetEntityByIndex( waterEntity );
                if( pwater && ( pwater->model != nullptr ) )
                {
                    waterDist += ( pwater->curstate.scale * 16 ); // Add in wave height
                }
            }
        }
        else
        {
            waterEntity = 0; // Don't need this in software
        }

        point = pparams->vieworg;

        // Eyes are above water, make sure we're above the waves
        if( pparams->waterlevel == WaterLevel::Waist )
        {
            point[2] -= waterDist;
            for( int i = 0; i < waterDist; i++ )
            {
                contents = gEngfuncs.PM_PointContents( point, nullptr );
                if( contents > CONTENTS_WATER )
                    break;
                point[2] += 1;
            }
            waterOffset = ( point[2] + waterDist ) - pparams->vieworg[2];
        }
        else
        {
            // eyes are under water.  Make sure we're far enough under
            point[2] += waterDist;

            for( int i = 0; i < waterDist; i++ )
            {
                contents = gEngfuncs.PM_PointContents( point, nullptr );
                if( contents <= CONTENTS_WATER )
                    break;
                point[2] -= 1;
            }
            waterOffset = ( point[2] - waterDist ) - pparams->vieworg[2];
        }
    }

    pparams->vieworg[2] += waterOffset;

    V_CalcViewRoll( pparams );

    V_AddIdle( pparams );

    // offsets
    angles = pparams->cl_viewangles;

    AngleVectors( angles, pparams->forward, pparams->right, pparams->up );

    // don't allow cheats in multiplayer
    if( pparams->maxclients <= 1 )
    {
        for( int i = 0; i < 3; i++ )
        {
            pparams->vieworg[i] += scr_ofsx->value * pparams->forward[i] + scr_ofsy->value * pparams->right[i] + scr_ofsz->value * pparams->up[i];
        }
    }

    // Treating cam_ofs[2] as the distance
    if( 0 != CL_IsThirdPerson() )
    {
        Vector ofs = g_vecZero;

        CL_CameraOffset( ofs );

        camAngles = ofs;
        camAngles[ROLL] = 0;

        AngleVectors( camAngles, camForward, camRight, camUp );

        for( int i = 0; i < 3; i++ )
        {
            pparams->vieworg[i] += -ofs[2] * camForward[i];
        }
    }

    // Give gun our viewangles
    view->angles = pparams->cl_viewangles;

    // set up gun position
    V_CalcGunAngle( pparams );

    // Use predicted origin as view origin.
    view->origin = pparams->simorg;
    view->origin[2] += ( waterOffset );
    view->origin = view->origin + pparams->viewheight;

    // Let the viewmodel shake at about 10% of the amplitude
    gEngfuncs.V_ApplyShake( view->origin, view->angles, 0.9 );

    for( int i = 0; i < 3; i++ )
    {
        view->origin[i] += bob * 0.4 * pparams->forward[i];
    }
    view->origin[2] += bob;

    // throw in a little tilt.
    view->angles[YAW] -= bob * 0.5;
    view->angles[ROLL] -= bob * 1;
    view->angles[PITCH] -= bob * 0.3;

    if( 0 != cl_bobtilt->value )
    {
        view->curstate.angles = view->angles;
    }

    // pushing the view origin down off of the same X/Z plane as the ent's origin will give the
    // gun a very nice 'shifting' effect when the player looks up/down. If there is a problem
    // with view model distortion, this may be a cause. (SJB).
    view->origin[2] -= 1;

    // fudge position around to keep amount of weapon visible
    // roughly equal with different FOV
    if( pparams->viewsize == 110 )
    {
        view->origin[2] += 1;
    }
    else if( pparams->viewsize == 100 )
    {
        view->origin[2] += 2;
    }
    else if( pparams->viewsize == 90 )
    {
        view->origin[2] += 1;
    }
    else if( pparams->viewsize == 80 )
    {
        view->origin[2] += 0.5;
    }

    // Add in the punchangle, if any
    pparams->viewangles = pparams->viewangles + pparams->punchangle;

    // Include client side punch, too
    pparams->viewangles = pparams->viewangles + ev_punchangle;

    V_DropPunchAngle( pparams->frametime, ev_punchangle );

    // smooth out stair step ups
#if 1
    if( 0 == pparams->smoothing && 0 != pparams->onground && pparams->simorg[2] - oldz > 0 )
    {
        float steptime;

        steptime = pparams->time - lasttime;
        if( steptime < 0 )
            // FIXME        I_Error ("steptime < 0");
            steptime = 0;

        oldz += steptime * 150;
        if( oldz > pparams->simorg[2] )
            oldz = pparams->simorg[2];
        if( pparams->simorg[2] - oldz > 18 )
            oldz = pparams->simorg[2] - 18;
        pparams->vieworg[2] += oldz - pparams->simorg[2];
        view->origin[2] += oldz - pparams->simorg[2];
    }
    else
    {
        oldz = pparams->simorg[2];
    }
#endif

    {
        static Vector lastorg;
        Vector delta;

        delta = pparams->simorg - lastorg;

        if( delta.Length() != 0.0 )
        {
            ViewInterp.Origins[ViewInterp.CurrentOrigin & ORIGIN_MASK] = pparams->simorg;
            ViewInterp.OriginTime[ViewInterp.CurrentOrigin & ORIGIN_MASK] = pparams->time;
            ViewInterp.CurrentOrigin++;

            lastorg = pparams->simorg;
        }
    }

    // Smooth out whole view in multiplayer when on trains, lifts
    if( cl_vsmoothing && 0 != cl_vsmoothing->value &&
        ( 0 != pparams->smoothing && ( pparams->maxclients > 1 ) ) )
    {
        int foundidx;
        float t;

        if( cl_vsmoothing->value < 0.0 )
        {
            gEngfuncs.Cvar_SetValue( "cl_vsmoothing", 0.0 );
        }

        t = pparams->time - cl_vsmoothing->value;

        int i;
        for( i = 1; i < ORIGIN_MASK; i++ )
        {
            foundidx = ViewInterp.CurrentOrigin - 1 - i;
            if( ViewInterp.OriginTime[foundidx & ORIGIN_MASK] <= t )
                break;
        }

        if( i < ORIGIN_MASK && ViewInterp.OriginTime[foundidx & ORIGIN_MASK] != 0.0 )
        {
            // Interpolate
            Vector delta;
            double frac;
            double dt;
            Vector neworg;

            dt = ViewInterp.OriginTime[( foundidx + 1 ) & ORIGIN_MASK] - ViewInterp.OriginTime[foundidx & ORIGIN_MASK];
            if( dt > 0.0 )
            {
                frac = ( t - ViewInterp.OriginTime[foundidx & ORIGIN_MASK] ) / dt;
                frac = std::min( 1.0, frac );
                delta = ViewInterp.Origins[( foundidx + 1 ) & ORIGIN_MASK] - ViewInterp.Origins[foundidx & ORIGIN_MASK];
                neworg = ViewInterp.Origins[foundidx & ORIGIN_MASK] + ( frac * delta );

                // Dont interpolate large changes
                if( delta.Length() < 64 )
                {
                    delta = neworg - pparams->simorg;

                    pparams->simorg = pparams->simorg + delta;
                    pparams->vieworg = pparams->vieworg + delta;
                    view->origin = view->origin + delta;
                }
            }
        }
    }

    // Store off v_angles before munging for third person
    v_angles = pparams->viewangles;
    v_client_aimangles = pparams->cl_viewangles;
    v_lastAngles = pparams->viewangles;
    //    v_cl_angles = pparams->cl_viewangles;    // keep old user mouse angles !
    if( 0 != CL_IsThirdPerson() )
    {
        pparams->viewangles = camAngles;
    }

    // Apply this at all times
    {
        float pitch = pparams->viewangles[0];

        // Normalize angles
        if( pitch > 180 )
            pitch -= 360.0;
        else if( pitch < -180 )
            pitch += 360;

        // Player pitch is inverted
        pitch /= -3.0;

        // Slam local player's pitch value
        ent->angles[0] = pitch;
        ent->curstate.angles[0] = pitch;
        ent->prevstate.angles[0] = pitch;
        ent->latched.prevangles[0] = pitch;
    }

    // override all previous settings if the viewent isn't the client
    if( pparams->viewentity > pparams->maxclients )
    {
        cl_entity_t* viewentity;
        viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
        if( viewentity )
        {
            pparams->vieworg = viewentity->origin;
            pparams->viewangles = viewentity->angles;

            // Store off overridden viewangles
            v_angles = pparams->viewangles;
        }
    }

    lasttime = pparams->time;

    v_origin = pparams->vieworg;
    v_crosshairangle = pparams->crosshairangle;
}

void V_SmoothInterpolateAngles( Vector& startAngle, Vector& endAngle, Vector& finalAngle, float degreesPerSec )
{
    float absd, frac, d, threshhold;

    NormalizeAngles( startAngle );
    NormalizeAngles( endAngle );

    for( int i = 0; i < 3; i++ )
    {
        d = endAngle[i] - startAngle[i];

        if( d > 180.0f )
        {
            d -= 360.0f;
        }
        else if( d < -180.0f )
        {
            d += 360.0f;
        }

        absd = fabs( d );

        if( absd > 0.01f )
        {
            frac = degreesPerSec * v_frametime;

            threshhold = degreesPerSec / 4;

            if( absd < threshhold )
            {
                float h = absd / threshhold;
                h *= h;
                frac *= h; // slow down last degrees
            }

            if( frac > absd )
            {
                finalAngle[i] = endAngle[i];
            }
            else
            {
                if( d > 0 )
                    finalAngle[i] = startAngle[i] + frac;
                else
                    finalAngle[i] = startAngle[i] - frac;
            }
        }
        else
        {
            finalAngle[i] = endAngle[i];
        }
    }

    NormalizeAngles( finalAngle );
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin( const Vector& angles, const Vector& origin, float distance, Vector& returnvec )
{
    Vector vecEnd;
    Vector forward;
    Vector vecStart;
    pmtrace_t* trace;
    int maxLoops = 8;

    int ignoreent = -1; // first, ignore no entity

    cl_entity_t* ent = nullptr;

    // Trace back from the target using the player's view angles
    AngleVectors( angles, &forward, nullptr, nullptr );

    forward = -forward;

    vecStart = origin;

    vecEnd = vecStart + ( distance * forward );

    while( maxLoops > 0 )
    {
        trace = gEngfuncs.PM_TraceLine( vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent );

        // WARNING! trace->ent is is the number in physent list not the normal entity number

        if( trace->ent <= 0 )
            break; // we hit the world or nothing, stop trace

        ent = gEngfuncs.GetEntityByIndex( PM_GetPhysEntInfo( trace->ent ) );

        if( ent == nullptr )
            break;

        // hit non-player solid BSP , stop here
        if( ent->curstate.solid == SOLID_BSP && 0 == ent->player )
            break;

        // if close enought to end pos, stop, otherwise continue trace
        if( ( vecEnd - trace->endpos ).Length() < 1.0f )
        {
            break;
        }
        else
        {
            ignoreent = trace->ent; // ignore last hit entity
            vecStart = trace->endpos;
        }

        maxLoops--;
    }

    /*    if (ent)
        {
            gEngfuncs.Con_Printf("Trace loops %i, entity %i, model %s, solid %i\n",
                (8-maxLoops),ent->curstate.number, ent->model->name, ent->curstate.solid);
        } */

    returnvec = trace->endpos + ( 4 * trace->plane.normal );

    v_lastDistance = ( origin - trace->endpos ).Length(); // real distance without offset
}

/*void V_GetDeathCam(cl_entity_t* ent1, cl_entity_t* ent2, Vector& angle, Vector& origin)
{
    Vector newAngle;
    Vector newOrigin;

    float distance = 168.0f;

    v_lastDistance += v_frametime * 96.0f;    // move unit per seconds back

    if (v_resetCamera)
        v_lastDistance = 64.0f;

    if (distance > v_lastDistance)
        distance = v_lastDistance;

    newOrigin = ent1->origin;

    if (ent1->player)
        newOrigin[2]+= 17; // head level of living player

    // get new angle towards second target
    if (ent2)
    {
        newAngle = ent2->origin - ent1->origin;
        VectorAngles(newAngle, newAngle);
        newAngle[0] = -newAngle[0];
    }
    else
    {
        // if no second target is given, look down to dead player
        newAngle[0] = 90.0f;
        newAngle[1] = 0.0f;
        newAngle[2] = 0;
    }

    // and smooth view
    V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);

    V_GetChaseOrigin(angle, newOrigin, distance, origin);

    v_lastAngles = angle;
}*/

void V_GetSingleTargetCam( cl_entity_t* ent1, Vector& angle, Vector& origin )
{
    Vector newAngle;
    Vector newOrigin;

    int flags = gHUD.m_Spectator.m_iObserverFlags;

    // see is target is a dead player
    const bool deadPlayer = 0 != ent1->player && ( ent1->curstate.solid == SOLID_NOT );

    float dfactor = ( flags & DRC_FLAG_DRAMATIC ) != 0 ? -1.0f : 1.0f;

    float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

    // go away in final scenes or if player just died
    if( ( flags & DRC_FLAG_FINAL ) != 0 )
        distance *= 2.0f;
    else if( deadPlayer )
        distance *= 1.5f;

    // let v_lastDistance float smoothly away
    v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

    if( distance > v_lastDistance )
        distance = v_lastDistance;

    newOrigin = ent1->origin;

    if( 0 != ent1->player )
    {
        if( deadPlayer )
            newOrigin[2] += 2; // laying on ground
        else
            newOrigin[2] += 17; // head level of living player
    }
    else
        newOrigin[2] += 8; // object, tricky, must be above bomb in CS

    // we have no second target, choose view direction based on
    // show front of primary target
    newAngle = ent1->angles;

    // show dead players from front, normal players back
    if( ( flags & DRC_FLAG_FACEPLAYER ) != 0 )
        newAngle[1] += 180.0f;


    newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

    // if final scene (bomb), show from real high pos
    if( ( flags & DRC_FLAG_FINAL ) != 0 )
        newAngle[0] = 22.5f;

    // choose side of object/player
    if( ( flags & DRC_FLAG_SIDE ) != 0 )
        newAngle[1] += 22.5f;
    else
        newAngle[1] -= 22.5f;

    V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

    // HACK, if player is dead don't clip against his dead body, can't check this
    V_GetChaseOrigin( angle, newOrigin, distance, origin );
}

float MaxAngleBetweenAngles( Vector& a1, Vector& a2 )
{
    float d, maxd = 0.0f;

    NormalizeAngles( a1 );
    NormalizeAngles( a2 );

    for( int i = 0; i < 3; i++ )
    {
        d = a2[i] - a1[i];
        if( d > 180 )
        {
            d -= 360;
        }
        else if( d < -180 )
        {
            d += 360;
        }

        d = fabs( d );

        if( d > maxd )
            maxd = d;
    }

    return maxd;
}

void V_GetDoubleTargetsCam( cl_entity_t* ent1, cl_entity_t* ent2, Vector& angle, Vector& origin )
{
    Vector newAngle;
    Vector newOrigin;
    Vector tempVec;

    int flags = gHUD.m_Spectator.m_iObserverFlags;

    float dfactor = ( flags & DRC_FLAG_DRAMATIC ) != 0 ? -1.0f : 1.0f;

    float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

    // go away in final scenes or if player just died
    if( ( flags & DRC_FLAG_FINAL ) != 0 )
        distance *= 2.0f;

    // let v_lastDistance float smoothly away
    v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

    if( distance > v_lastDistance )
        distance = v_lastDistance;

    newOrigin = ent1->origin;

    if( 0 != ent1->player )
        newOrigin[2] += 17; // head level of living player
    else
        newOrigin[2] += 8; // object, tricky, must be above bomb in CS

    // get new angle towards second target
    newAngle = ent2->origin - ent1->origin;

    VectorAngles( newAngle, newAngle );
    newAngle[0] = -newAngle[0];

    // set angle diffrent in Dramtaic scenes
    newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

    if( ( flags & DRC_FLAG_SIDE ) != 0 )
        newAngle[1] += 22.5f;
    else
        newAngle[1] -= 22.5f;

    float d = MaxAngleBetweenAngles( v_lastAngles, newAngle );

    if( ( d < v_cameraFocusAngle ) && ( v_cameraMode == CAM_MODE_RELAX ) )
    {
        // difference is to small and we are in relax camera mode, keep viewangles
        newAngle = v_lastAngles;
    }
    else if( ( d < v_cameraRelaxAngle ) && ( v_cameraMode == CAM_MODE_FOCUS ) )
    {
        // we catched up with our target, relax again
        v_cameraMode = CAM_MODE_RELAX;
    }
    else
    {
        // target move too far away, focus camera again
        v_cameraMode = CAM_MODE_FOCUS;
    }

    // and smooth view, if not a scene cut
    if( v_resetCamera || ( v_cameraMode == CAM_MODE_RELAX ) )
    {
        angle = newAngle;
    }
    else
    {
        V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 180.0f );
    }

    V_GetChaseOrigin( newAngle, newOrigin, distance, origin );

    // move position up, if very close at target
    if( v_lastDistance < 64.0f )
        origin[2] += 16.0f * ( 1.0f - ( v_lastDistance / 64.0f ) );

    // calculate angle to second target
    tempVec = ent2->origin - origin;
    VectorAngles( tempVec, tempVec );
    tempVec[0] = -tempVec[0];

    /* take middle between two viewangles
    InterpolateAngles( newAngle, tempVec, newAngle, 0.5f); */
}

void V_GetDirectedChasePosition( cl_entity_t* ent1, cl_entity_t* ent2, Vector& angle, Vector& origin )
{

    if( v_resetCamera )
    {
        v_lastDistance = 4096.0f;
        // v_cameraMode = CAM_MODE_FOCUS;
    }

    if( ( ent2 == ( cl_entity_t* )0xFFFFFFFF ) || ( 0 != ent1->player && ( ent1->curstate.solid == SOLID_NOT ) ) )
    {
        // we have no second target or player just died
        V_GetSingleTargetCam( ent1, angle, origin );
    }
    else if( ent2 )
    {
        // keep both target in view
        V_GetDoubleTargetsCam( ent1, ent2, angle, origin );
    }
    else
    {
        // second target disappeard somehow (dead)

        // keep last good viewangle
        Vector newOrigin;

        int flags = gHUD.m_Spectator.m_iObserverFlags;

        float dfactor = ( flags & DRC_FLAG_DRAMATIC ) != 0 ? -1.0f : 1.0f;

        float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

        // go away in final scenes or if player just died
        if( ( flags & DRC_FLAG_FINAL ) != 0 )
            distance *= 2.0f;

        // let v_lastDistance float smoothly away
        v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

        if( distance > v_lastDistance )
            distance = v_lastDistance;

        newOrigin = ent1->origin;

        if( 0 != ent1->player )
            newOrigin[2] += 17; // head level of living player
        else
            newOrigin[2] += 8; // object, tricky, must be above bomb in CS

        V_GetChaseOrigin( angle, newOrigin, distance, origin );
    }

    v_lastAngles = angle;
}

void V_GetChasePos( int target, Vector* cl_angles, Vector& origin, Vector& angles )
{
    cl_entity_t* ent = nullptr;

    if( 0 != target )
    {
        ent = gEngfuncs.GetEntityByIndex( target );
    }

    if( !ent )
    {
        // just copy a save in-map position
        angles = vJumpAngles;
        origin = vJumpOrigin;
        return;
    }



    if( 0 != gHUD.m_Spectator.m_autoDirector->value )
    {
        if( 0 != g_iUser3 )
            V_GetDirectedChasePosition( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ),
                angles, origin );
        else
            V_GetDirectedChasePosition( ent, ( cl_entity_t* )0xFFFFFFFF,
                angles, origin );
    }
    else
    {
        if( cl_angles == nullptr ) // no mouse angles given, use entity angles ( locked mode )
        {
            angles = ent->angles;
            angles[0] *= -1;
        }
        else
            angles = *cl_angles;


        origin = ent->origin;

        origin = origin + VEC_VIEW; // some offset

        V_GetChaseOrigin( angles, origin, cl_chasedist->value, origin );
    }

    v_resetCamera = false;
}

void V_ResetChaseCam()
{
    v_resetCamera = true;
}


void V_GetInEyePos( int target, Vector& origin, Vector& angles )
{
    if( 0 == target )
    {
        // just copy a save in-map position
        angles = vJumpAngles;
        origin = vJumpOrigin;
        return;
    }


    cl_entity_t* ent = gEngfuncs.GetEntityByIndex( target );

    if( !ent )
        return;

    origin = ent->origin;
    angles = ent->angles;

    angles[PITCH] *= -3.0f; // see CL_ProcessEntityUpdate()

    if( ent->curstate.solid == SOLID_NOT )
    {
        angles[ROLL] = 80; // dead view angle
        origin = origin + VEC_DEAD_VIEW;
    }
    else if( ent->curstate.usehull == 1 )
    {
        origin = origin + VEC_DUCK_VIEW;
    }
    else
        // exacty eye position can't be caluculated since it depends on
        // client values like cl_bobcycle, this offset matches the default values
        origin = origin + VEC_VIEW;
}

void V_GetMapFreePosition( Vector& cl_angles, Vector& origin, Vector& angles )
{
    Vector forward;
    Vector zScaledTarget;

    angles = cl_angles;

    // modify angles since we don't wanna see map's bottom
    angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );

    zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
    zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
    zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * ( ( 90.0f - angles[0] ) / 90.0f );


    AngleVectors( angles, &forward, nullptr, nullptr );

    VectorNormalize( forward );

    origin = zScaledTarget + ( -( 4096.0f / gHUD.m_Spectator.m_mapZoom ) * forward );
}

void V_GetMapChasePosition( int target, Vector& cl_angles, Vector& origin, Vector& angles )
{
    Vector forward;

    if( 0 != target )
    {
        cl_entity_t* ent = gEngfuncs.GetEntityByIndex( target );

        if( 0 != gHUD.m_Spectator.m_autoDirector->value )
        {
            // this is done to get the angles made by director mode
            V_GetChasePos( target, &cl_angles, origin, angles );
            origin = ent->origin;

            // keep fix chase angle horizontal
            angles[0] = 45.0f;
        }
        else
        {
            angles = cl_angles;
            origin = ent->origin;

            // modify angles since we don't wanna see map's bottom
            angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
        }
    }
    else
    {
        // keep out roaming position, but modify angles
        angles = cl_angles;
        angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
    }

    origin[2] *= ( ( 90.0f - angles[0] ) / 90.0f );
    angles[2] = 0.0f; // don't roll angle (if chased player is dead)

    AngleVectors( angles, &forward, nullptr, nullptr );

    VectorNormalize( forward );

    origin = origin + ( -1536 * forward );
}

int V_FindViewModelByWeaponModel( int weaponindex )
{

    static const char* modelmap[][2] = {

        {"models/p_crossbow.mdl", "models/v_crossbow.mdl"},
        {"models/p_crowbar.mdl", "models/v_crowbar.mdl"},
        {"models/p_egon.mdl", "models/v_egon.mdl"},
        {"models/p_gauss.mdl", "models/v_gauss.mdl"},
        {"models/p_9mmhandgun.mdl", "models/v_9mmhandgun.mdl"},
        {"models/p_grenade.mdl", "models/v_grenade.mdl"},
        {"models/p_hgun.mdl", "models/v_hgun.mdl"},
        {"models/p_9mmAR.mdl", "models/v_9mmAR.mdl"},
        {"models/p_357.mdl", "models/v_357.mdl"},
        {"models/p_rpg.mdl", "models/v_rpg.mdl"},
        {"models/p_shotgun.mdl", "models/v_shotgun.mdl"},
        {"models/p_squeak.mdl", "models/v_squeak.mdl"},
        {"models/p_tripmine.mdl", "models/v_tripmine.mdl"},
        {"models/p_satchel_radio.mdl", "models/v_satchel_radio.mdl"},
        {"models/p_satchel.mdl", "models/v_satchel.mdl"},
        {nullptr, nullptr}};

    model_t* weaponModel = IEngineStudio.GetModelByIndex( weaponindex );

    if( weaponModel )
    {
        int len = strlen( weaponModel->name );
        int i = 0;

        while( modelmap[i] != nullptr )
        {
            if( !strnicmp( weaponModel->name, modelmap[i][0], len ) )
            {
                return gEngfuncs.pEventAPI->EV_FindModelIndex( modelmap[i][1] );
            }
            i++;
        }

        return 0;
    }
    else
        return 0;
}


/*
==================
V_CalcSpectatorRefdef

==================
*/
void V_CalcSpectatorRefdef( ref_params_t* pparams )
{
    static Vector velocity( 0.0f, 0.0f, 0.0f );

    static int lastWeaponModelIndex = 0;
    static int lastViewModelIndex = 0;

    cl_entity_t* ent = gEngfuncs.GetEntityByIndex( g_iUser2 );

    pparams->onlyClientDraw = 0;

    // refresh position
    v_sim_org = pparams->simorg;

    // get old values
    v_cl_angles = pparams->cl_viewangles;
    v_angles = pparams->viewangles;
    v_origin = pparams->vieworg;

    if( ( g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE ) && ent )
    {
        // calculate player velocity
        float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

        if( timeDiff > 0 )
        {
            Vector distance;
            distance = ent->prevstate.origin - ent->curstate.origin;
            distance = distance * ( 1 / timeDiff );

            velocity[0] = velocity[0] * 0.9f + distance[0] * 0.1f;
            velocity[1] = velocity[1] * 0.9f + distance[1] * 0.1f;
            velocity[2] = velocity[2] * 0.9f + distance[2] * 0.1f;

            pparams->simvel = velocity;
        }

        // predict missing client data and set weapon model ( in HLTV mode or inset in eye mode )
        if( 0 != gEngfuncs.IsSpectateOnly() )
        {
            V_GetInEyePos( g_iUser2, pparams->simorg, pparams->cl_viewangles );

            pparams->health = 1;

            cl_entity_t* gunModel = gEngfuncs.GetViewModel();

            if( lastWeaponModelIndex != ent->curstate.weaponmodel )
            {
                // weapon model changed

                lastWeaponModelIndex = ent->curstate.weaponmodel;
                lastViewModelIndex = V_FindViewModelByWeaponModel( lastWeaponModelIndex );
                if( 0 != lastViewModelIndex )
                {
                    gEngfuncs.pfnWeaponAnim( 0, 0 ); // reset weapon animation
                }
                else
                {
                    // model not found
                    gunModel->model = nullptr; // disable weapon model
                    lastWeaponModelIndex = lastViewModelIndex = 0;
                }
            }

            if( 0 != lastViewModelIndex )
            {
                gunModel->model = IEngineStudio.GetModelByIndex( lastViewModelIndex );
                gunModel->curstate.modelindex = lastViewModelIndex;
                gunModel->curstate.frame = 0;
                gunModel->curstate.colormap = 0;
                gunModel->index = g_iUser2;
            }
            else
            {
                gunModel->model = nullptr; // disable weaopn model
            }
        }
        else
        {
            // only get viewangles from entity
            pparams->cl_viewangles = ent->angles;
            pparams->cl_viewangles[PITCH] *= -3.0f; // see CL_ProcessEntityUpdate()
        }
    }

    v_frametime = pparams->frametime;

    if( pparams->nextView == 0 )
    {
        // first renderer cycle, full screen

        switch ( g_iUser1 )
        {
        case OBS_CHASE_LOCKED:
            V_GetChasePos( g_iUser2, nullptr, v_origin, v_angles );
            break;

        case OBS_CHASE_FREE:
            V_GetChasePos( g_iUser2, &v_cl_angles, v_origin, v_angles );
            break;

        case OBS_ROAMING:
            v_angles = v_cl_angles;
            v_origin = v_sim_org;

            // override values if director is active
            gHUD.m_Spectator.GetDirectorCamera( v_origin, v_angles );
            break;

        case OBS_IN_EYE:
            V_CalcNormalRefdef( pparams );
            break;

        case OBS_MAP_FREE:
            pparams->onlyClientDraw = 1;
            V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
            break;

        case OBS_MAP_CHASE:
            pparams->onlyClientDraw = 1;
            V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );
            break;
        }

        if( 0 != gHUD.m_Spectator.m_pip->value )
            pparams->nextView = 1; // force a second renderer view

        gHUD.m_Spectator.m_iDrawCycle = 0;
    }
    else
    {
        // second renderer cycle, inset window

        // set inset parameters
        pparams->viewport[0] = XRES( gHUD.m_Spectator.m_OverviewData.insetWindowX ); // change viewport to inset window
        pparams->viewport[1] = YRES( gHUD.m_Spectator.m_OverviewData.insetWindowY );
        pparams->viewport[2] = XRES( gHUD.m_Spectator.m_OverviewData.insetWindowWidth );
        pparams->viewport[3] = YRES( gHUD.m_Spectator.m_OverviewData.insetWindowHeight );
        pparams->nextView = 0; // on further view

        // override some settings in certain modes
        switch ( (int)gHUD.m_Spectator.m_pip->value )
        {
        case INSET_CHASE_FREE:
            V_GetChasePos( g_iUser2, &v_cl_angles, v_origin, v_angles );
            break;

        case INSET_IN_EYE:
            V_CalcNormalRefdef( pparams );
            break;

        case INSET_MAP_FREE:
            pparams->onlyClientDraw = 1;
            V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
            break;

        case INSET_MAP_CHASE:
            pparams->onlyClientDraw = 1;

            if( g_iUser1 == OBS_ROAMING )
                V_GetMapChasePosition( 0, v_cl_angles, v_origin, v_angles );
            else
                V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );

            break;
        }

        gHUD.m_Spectator.m_iDrawCycle = 1;
    }

    // write back new values into pparams
    pparams->cl_viewangles = v_cl_angles;
    pparams->viewangles = v_angles;
    pparams->vieworg = v_origin;
}



void DLLEXPORT V_CalcRefdef( ref_params_t* pparams )
{
    // This is the earliest the engine will unconditionally call into the client
    // after the connection state has changed to active and signon has reached state 2.
    g_Client.ClientActivated();

    // Configure fog parameters before anything is drawn to ensure that the engine checks fog state based on these parameters.
    RenderFog();

    g_Paused = pparams->paused != 0;
    g_WaterLevel = pparams->waterlevel;

    // intermission / finale rendering
    if( 0 != pparams->intermission )
    {
        V_CalcIntermissionRefdef( pparams );
    }
    else if( 0 != pparams->spectator || 0 != g_iUser1 ) // g_iUser true if in spectator mode
    {
        V_CalcSpectatorRefdef( pparams );
    }
    else if( 0 == pparams->paused )
    {
        V_CalcNormalRefdef( pparams );
    }

    gHUD.m_Speedometer.SetSpeed( pparams->simvel.Length2D() );

    g_ViewEntity = pparams->viewentity;

    /*
    // Example of how to overlay the whole screen with red at 50 % alpha
    #define SF_TEST
    #if defined SF_TEST
        {
            screenfade_t sf;
            gEngfuncs.pfnGetScreenFade( &sf );

            sf.fader = 255;
            sf.fadeg = 0;
            sf.fadeb = 0;
            sf.fadealpha = 128;
            sf.fadeFlags = FFADE_STAYOUT | FFADE_OUT;

            gEngfuncs.pfnSetScreenFade( &sf );
        }
    #endif
    */
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle( float frametime, Vector& punchangle )
{
    float len;

    len = VectorNormalize( punchangle );
    len -= ( 10.0 + len * 0.5 ) * frametime;
    len = std::max( len, 0.0f );
    punchangle = punchangle * len;
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis( int axis, float punch )
{
    ev_punchangle[axis] = punch;
}

/*
=============
V_Init
=============
*/
void V_Init()
{
    gEngfuncs.pfnAddCommand( "centerview", V_StartPitchDrift );

    scr_ofsx = gEngfuncs.pfnRegisterVariable( "scr_ofsx", "0", 0 );
    scr_ofsy = gEngfuncs.pfnRegisterVariable( "scr_ofsy", "0", 0 );
    scr_ofsz = gEngfuncs.pfnRegisterVariable( "scr_ofsz", "0", 0 );

    v_centermove = gEngfuncs.pfnRegisterVariable( "v_centermove", "0.15", 0 );
    v_centerspeed = gEngfuncs.pfnRegisterVariable( "v_centerspeed", "500", 0 );

    cl_bobcycle = gEngfuncs.pfnRegisterVariable( "cl_bobcycle", "0.8", 0 ); // best default for my experimental gun wag (sjb)
    cl_bob = gEngfuncs.pfnRegisterVariable( "cl_bob", "0.01", 0 );          // best default for my experimental gun wag (sjb)
    cl_bobup = gEngfuncs.pfnRegisterVariable( "cl_bobup", "0.5", 0 );
    cl_waterdist = gEngfuncs.pfnRegisterVariable( "cl_waterdist", "4", 0 );
    cl_chasedist = gEngfuncs.pfnRegisterVariable( "cl_chasedist", "112", 0 );
}


// #define TRACE_TEST
#if defined( TRACE_TEST )

extern float in_fov;
/*
====================
CalcFov
====================
*/
float CalcFov( float fov_x, float width, float height )
{
    float a;
    float x;

    if( fov_x < 1 || fov_x > 179 )
        fov_x = 90; // error, set to 90

    x = width / tan( fov_x / 360 * PI );

    a = atan( height / x );

    a = a * 360 / PI;

    return a;
}

int hitent = -1;

void V_Move( int mx, int my )
{
    float fov;
    float fx, fy;
    float dx, dy;
    float c_x, c_y;
    float dX, dY;
    Vector forward, up, right;
    Vector newangles;

    Vector farpoint;
    pmtrace_t tr;

    fov = CalcFov( in_fov, (float)ScreenWidth, (float)ScreenHeight );

    c_x = (float)ScreenWidth / 2.0;
    c_y = (float)ScreenHeight / 2.0;

    dx = (float)mx - c_x;
    dy = (float)my - c_y;

    // Proportion we moved in each direction
    fx = dx / c_x;
    fy = dy / c_y;

    dX = fx * in_fov / 2.0;
    dY = fy * fov / 2.0;

    newangles = v_angles;

    newangles[YAW] -= dX;
    newangles[PITCH] += dY;

    // Now rotate v_forward around that point
    AngleVectors( newangles, forward, right, up );

    farpoint = v_origin + 8192 * forward;

    // Trace
    tr = *(gEngfuncs.PM_TraceLine(v_origin, farpoint, PM_TRACELINE_PHYSENTSONLY, 2 /*point sized hull*/, -1));

    if( tr.fraction != 1.0 && tr.ent != 0 )
    {
        hitent = PM_GetPhysEntInfo( tr.ent );
        PM_ParticleLine( v_origin, tr.endpos, 5, 1.0, 0.0 );
    }
    else
    {
        hitent = -1;
    }
}

#endif
