//========= Copyright ï¿½ 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// There are hud.h's coming out of the woodwork so this ensures that we get the right one.
#include "hud.h"

#include "demo.h"
#include "demo_api.h"
#include "voice_status.h"
#include "r_efx.h"
#include "entity_types.h"
#include "VGUI_ActionSignal.h"
#include "VGUI_Scheme.h"
#include "VGUI_TextImage.h"
#include "vgui_loadtga.h"
#include "vgui_helpers.h"
#include "VGUI_MouseCode.h"



using namespace vgui;


extern bool cam_thirdperson;


#define VOICE_MODEL_INTERVAL 0.3
#define SCOREBOARD_BLINK_FREQUENCY 0.3 // How often to blink the scoreboard icons.
#define SQUELCHOSCILLATE_PER_SECOND 2.0f

// ---------------------------------------------------------------------- //
// The voice manager for the client.
// ---------------------------------------------------------------------- //
CVoiceStatus g_VoiceStatus;

CVoiceStatus* GetClientVoiceMgr()
{
    return &g_VoiceStatus;
}



// ---------------------------------------------------------------------- //
// CVoiceStatus.
// ---------------------------------------------------------------------- //

static CVoiceStatus* g_pInternalVoiceStatus = nullptr;

int g_BannedPlayerPrintCount;
void ForEachBannedPlayer( char id[16] )
{
    char str[256];
    sprintf( str, "Ban %d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X\n",
        g_BannedPlayerPrintCount++,
        id[0], id[1], id[2], id[3],
        id[4], id[5], id[6], id[7],
        id[8], id[9], id[10], id[11],
        id[12], id[13], id[14], id[15] );

    gEngfuncs.pfnConsolePrint( str );
}


void ShowBannedCallback()
{
    if( g_pInternalVoiceStatus )
    {
        g_BannedPlayerPrintCount = 0;
        gEngfuncs.pfnConsolePrint( "------- BANNED PLAYERS -------\n" );
        g_pInternalVoiceStatus->m_BanMgr.ForEachBannedPlayer( ForEachBannedPlayer );
        gEngfuncs.pfnConsolePrint( "------------------------------\n" );
    }
}


// ---------------------------------------------------------------------- //
// CVoiceStatus.
// ---------------------------------------------------------------------- //

CVoiceStatus::CVoiceStatus()
{
    m_bBanMgrInitialized = false;
    m_LastUpdateServerState = 0;

    m_pSpeakerLabelIcon = nullptr;
    m_pScoreboardNeverSpoken = nullptr;
    m_pScoreboardNotSpeaking = nullptr;
    m_pScoreboardSpeaking = nullptr;
    m_pScoreboardSpeaking2 = nullptr;
    m_pScoreboardSquelch = nullptr;
    m_pScoreboardBanned = nullptr;

    m_pLocalBitmap = nullptr;
    m_pAckBitmap = nullptr;

    m_bTalking = m_bServerAcked = false;

    memset( m_pBanButtons, 0, sizeof( m_pBanButtons ) );

    m_pParentPanel = nullptr;

    m_bServerModEnable = -1;
}


CVoiceStatus::~CVoiceStatus()
{
    g_pInternalVoiceStatus = nullptr;

    for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
    {
        delete m_Labels[i].m_pLabel;
        m_Labels[i].m_pLabel = nullptr;

        delete m_Labels[i].m_pIcon;
        m_Labels[i].m_pIcon = nullptr;

        delete m_Labels[i].m_pBackground;
        m_Labels[i].m_pBackground = nullptr;
    }

    delete m_pLocalLabel;
    m_pLocalLabel = nullptr;

    FreeBitmaps();
}


int CVoiceStatus::Init( 
    IVoiceStatusHelper* pHelper,
    Panel** pParentPanel )
{
    // Setup the voice_modenable cvar.
    gEngfuncs.pfnRegisterVariable( "voice_modenable", "1", FCVAR_ARCHIVE );

    gEngfuncs.pfnRegisterVariable( "voice_clientdebug", "0", 0 );

    gEngfuncs.pfnAddCommand( "voice_showbanned", ShowBannedCallback );

    m_BanMgr.Init();
    m_bBanMgrInitialized = true;

    assert( !g_pInternalVoiceStatus );
    g_pInternalVoiceStatus = this;

    m_BlinkTimer = 0;
    m_VoiceHeadModel = 0;
    memset( m_Labels, 0, sizeof( m_Labels ) );

    for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
    {
        CVoiceLabel* pLabel = &m_Labels[i];

        pLabel->m_pBackground = new Label( "" );

        if( pLabel->m_pLabel = new Label( "" ); pLabel->m_pLabel )
        {
            pLabel->m_pLabel->setVisible( true );
            pLabel->m_pLabel->setFont( Scheme::sf_primary2 );
            pLabel->m_pLabel->setTextAlignment( Label::a_east );
            pLabel->m_pLabel->setContentAlignment( Label::a_east );
            pLabel->m_pLabel->setParent( pLabel->m_pBackground );
        }

        if( pLabel->m_pIcon = new ImagePanel( nullptr ); pLabel->m_pIcon )
        {
            pLabel->m_pIcon->setVisible( true );
            pLabel->m_pIcon->setParent( pLabel->m_pBackground );
        }

        pLabel->m_clientindex = -1;
    }

    m_pLocalLabel = new ImagePanel( nullptr );

    m_bInSquelchMode = false;

    m_pHelper = pHelper;
    m_pParentPanel = pParentPanel;
    gHUD.AddHudElem( this );
    m_iFlags = HUD_ACTIVE;

    g_ClientUserMessages.RegisterHandler( "VoiceMask", &CVoiceStatus::HandleVoiceMaskMsg, this );
    g_ClientUserMessages.RegisterHandler( "ReqState", &CVoiceStatus::HandleReqStateMsg, this );

    return 1;
}

void CVoiceStatus::Shutdown()
{
    if( m_bBanMgrInitialized )
    {
        m_BanMgr.SaveState();
        m_bBanMgrInitialized = false;
    }
}

bool CVoiceStatus::VidInit()
{
    FreeBitmaps();


    if( m_pLocalBitmap = vgui_LoadTGA( "gfx/vgui/icntlk_pl.tga" ); m_pLocalBitmap )
    {
        m_pLocalBitmap->setColor( Color( 255, 255, 255, 135 ) );
    }

    if( m_pAckBitmap = vgui_LoadTGA( "gfx/vgui/icntlk_sv.tga" ); m_pAckBitmap )
    {
        m_pAckBitmap->setColor( Color( 255, 255, 255, 135 ) ); // Give just a tiny bit of translucency so software draws correctly.
    }

    m_pLocalLabel->setImage( m_pLocalBitmap );
    m_pLocalLabel->setVisible( false );


    if( m_pSpeakerLabelIcon = vgui_LoadTGA( "gfx/vgui/speaker4.tga", false ); m_pSpeakerLabelIcon )
        m_pSpeakerLabelIcon->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardNeverSpoken = vgui_LoadTGA( "gfx/vgui/640_speaker1.tga", false ); m_pScoreboardNeverSpoken )
        m_pScoreboardNeverSpoken->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardNotSpeaking = vgui_LoadTGA( "gfx/vgui/640_speaker2.tga", false ); m_pScoreboardNotSpeaking )
        m_pScoreboardNotSpeaking->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardSpeaking = vgui_LoadTGA( "gfx/vgui/640_speaker3.tga", false ); m_pScoreboardSpeaking )
        m_pScoreboardSpeaking->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardSpeaking2 = vgui_LoadTGA( "gfx/vgui/640_speaker4.tga", false ); m_pScoreboardSpeaking2 )
        m_pScoreboardSpeaking2->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardSquelch = vgui_LoadTGA( "gfx/vgui/icntlk_squelch.tga" ); m_pScoreboardSquelch )
        m_pScoreboardSquelch->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    if( m_pScoreboardBanned = vgui_LoadTGA( "gfx/vgui/640_voiceblocked.tga" ); m_pScoreboardBanned )
        m_pScoreboardBanned->setColor( Color( 255, 255, 255, 1 ) ); // Give just a tiny bit of translucency so software draws correctly.

    // Figure out the voice head model height.
    m_VoiceHeadModelHeight = 45;
    const auto fileContents = FileSystem_LoadFileIntoBuffer( "scripts/voicemodel.txt", FileContentFormat::Text );
    if( !fileContents.empty() )
    {
        // TODO: token can potentially be larger than buffer.
        char token[4096];
        gEngfuncs.COM_ParseFile( reinterpret_cast<const char*>( fileContents.data() ), token );
        if( token[0] >= '0' && token[0] <= '9' )
        {
            m_VoiceHeadModelHeight = (float)atof( token );
        }
    }

    m_VoiceHeadModel = gEngfuncs.pfnSPR_Load( "sprites/voiceicon.spr" );
    return true;
}


void CVoiceStatus::Frame( double frametime )
{
    // check server banned players once per second
    if( gEngfuncs.GetClientTime() - m_LastUpdateServerState > 1 )
    {
        UpdateServerState( false );
    }

    m_BlinkTimer += frametime;

    // Update speaker labels.
    if( m_pHelper->CanShowSpeakerLabels() )
    {
        for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
            m_Labels[i].m_pBackground->setVisible( m_Labels[i].m_clientindex != -1 );
    }
    else
    {
        for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
            m_Labels[i].m_pBackground->setVisible( false );
    }

    for( int i = 0; i < MAX_PLAYERS; i++ )
        UpdateBanButton( i );
}


void CVoiceStatus::CreateEntities()
{
    if( 0 == m_VoiceHeadModel )
        return;

    cl_entity_t* localPlayer = gEngfuncs.GetLocalPlayer();

    int iOutModel = 0;
    for( int i = 0; i < MAX_PLAYERS; i++ )
    {
        if( !m_VoicePlayers[i] )
            continue;

        cl_entity_t* pClient = gEngfuncs.GetEntityByIndex( i + 1 );

        // Don't show an icon if the player is not in our PVS.
        if( !pClient || pClient->curstate.messagenum < localPlayer->curstate.messagenum )
            continue;

        // Don't show an icon for dead or spectating players (ie: invisible entities).
        if( ( pClient->curstate.effects & EF_NODRAW ) != 0 )
            continue;

        // Don't show an icon for the local player unless we're in thirdperson mode.
        if( pClient == localPlayer && !cam_thirdperson )
            continue;

        cl_entity_t* pEnt = &m_VoiceHeadModels[iOutModel];
        ++iOutModel;

        memset( pEnt, 0, sizeof( *pEnt ) );

        pEnt->curstate.rendermode = kRenderTransAdd;
        pEnt->curstate.renderamt = 255;
        pEnt->baseline.renderamt = 255;
        pEnt->curstate.renderfx = kRenderFxNoDissipation;
        pEnt->curstate.framerate = 1;
        pEnt->curstate.frame = 0;
        pEnt->model = ( model_t* )gEngfuncs.GetSpritePointer( m_VoiceHeadModel );
        pEnt->angles[0] = pEnt->angles[1] = pEnt->angles[2] = 0;
        pEnt->curstate.scale = 0.5f;

        pEnt->origin[0] = pEnt->origin[1] = 0;
        pEnt->origin[2] = 45;

        pEnt->origin = pEnt->origin + pClient->origin;

        // Tell the engine.
        gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, pEnt );
    }
}


void CVoiceStatus::UpdateSpeakerStatus( int entindex, bool bTalking )
{
    cvar_t* pVoiceLoopback = nullptr;

    if( !m_pParentPanel || !*m_pParentPanel )
    {
        return;
    }

    if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
    {
        char msg[256];
        snprintf( msg, sizeof( msg ), "CVoiceStatus::UpdateSpeakerStatus: ent %d talking = %d\n", entindex, static_cast<int>( bTalking ) );
        gEngfuncs.pfnConsolePrint( msg );
    }

    int iLocalPlayerIndex = gEngfuncs.GetLocalPlayer()->index;

    // Is it the local player talking?
    if( entindex == -1 )
    {
        m_bTalking = bTalking;
        if( bTalking )
        {
            // Enable voice for them automatically if they try to talk.
            gEngfuncs.pfnClientCmd( "voice_modenable 1" );
        }

        // now set the player index to the correct index for the local player
        // this will allow us to have the local player's icon flash in the scoreboard
        entindex = iLocalPlayerIndex;

        pVoiceLoopback = gEngfuncs.pfnGetCvarPointer( "voice_loopback" );
    }
    else if( entindex == -2 )
    {
        m_bServerAcked = bTalking;
    }

    if( entindex >= 0 && entindex <= MAX_PLAYERS )
    {
        int iClient = entindex - 1;
        if( iClient < 0 )
        {
            return;
        }

        CVoiceLabel* pLabel = FindVoiceLabel( iClient );
        if( bTalking )
        {
            m_VoicePlayers[iClient] = true;
            m_VoiceEnabledPlayers[iClient] = true;

            // If we don't have a label for this guy yet, then create one.
            if( !pLabel )
            {
                // if this isn't the local player (unless they have voice_loopback on)
                if( ( entindex != iLocalPlayerIndex ) || ( pVoiceLoopback && 0 != pVoiceLoopback->value ) )
                {
                    if( pLabel = GetFreeVoiceLabel(); pLabel )
                    {
                        // Get the name from the engine.
                        hud_player_info_t info;
                        memset( &info, 0, sizeof( info ) );
                        gEngfuncs.pfnGetPlayerInfo( entindex, &info );

                        char paddedName[512];
                        snprintf( paddedName, sizeof( paddedName ), "%s   ", info.name );

                        int color[3];
                        m_pHelper->GetPlayerTextColor( entindex, color );

                        if( pLabel->m_pBackground )
                        {
                            pLabel->m_pBackground->setBgColor( color[0], color[1], color[2], 135 );
                            pLabel->m_pBackground->setParent( *m_pParentPanel );
                            pLabel->m_pBackground->setVisible( m_pHelper->CanShowSpeakerLabels() );
                        }

                        if( pLabel->m_pLabel )
                        {
                            pLabel->m_pLabel->setFgColor( 255, 255, 255, 0 );
                            pLabel->m_pLabel->setBgColor( 0, 0, 0, 255 );
                            pLabel->m_pLabel->setText( "%s", paddedName );
                        }

                        pLabel->m_clientindex = iClient;
                    }
                }
            }
        }
        else
        {
            m_VoicePlayers[iClient] = false;

            // If we have a label for this guy, kill it.
            if( pLabel )
            {
                pLabel->m_pBackground->setVisible( false );
                pLabel->m_clientindex = -1;
            }
        }
    }

    RepositionLabels();
}


void CVoiceStatus::UpdateServerState( bool bForce )
{
    // Can't do anything when we're not in a level.
    char const* pLevelName = gEngfuncs.pfnGetLevelName();
    if( pLevelName[0] == 0 )
    {
        if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
        {
            gEngfuncs.pfnConsolePrint( "CVoiceStatus::UpdateServerState: pLevelName[0]==0\n" );
        }

        return;
    }

    int bCVarModEnable = static_cast<int>( 0 != gEngfuncs.pfnGetCvarFloat( "voice_modenable" ) );
    if( bForce || m_bServerModEnable != bCVarModEnable )
    {
        m_bServerModEnable = bCVarModEnable;

        char str[64];
        snprintf( str, sizeof( str ), "vmodenable %d", int( m_bServerModEnable ) );
        ServerCmd( str );

        if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
        {
            char msg[256];
            sprintf( msg, "CVoiceStatus::UpdateServerState: Sending '%s'\n", str );
            gEngfuncs.pfnConsolePrint( msg );
        }
    }

    char str[2048];
    sprintf( str, "vban" );
    bool bChange = false;

    for( unsigned long dw = 0; dw < VOICE_MAX_PLAYERS_DW; dw++ )
    {
        // The ban mask is a 32 bit int, so make sure this doesn't silently break.
        // Note that the server will also need updating.
        static_assert( MAX_PLAYERS <= 32, "The voice ban bit vector only supports up to 32 players" );

        unsigned long serverBanMask = 0;
        unsigned long banMask = 0;
        for( unsigned long i = 0; i < MAX_PLAYERS; i++ )
        {
            char playerID[16];
            if( 0 == gEngfuncs.GetPlayerUniqueID( i + 1, playerID ) )
                continue;

            if( m_BanMgr.GetPlayerBan( playerID ) )
                banMask |= 1 << i;

            if( m_ServerBannedPlayers[dw * MAX_PLAYERS + i] )
                serverBanMask |= 1 << i;
        }

        if( serverBanMask != banMask )
            bChange = true;

        // Ok, the server needs to be updated.
        char numStr[512];
        sprintf( numStr, " %lx", banMask );
        strcat( str, numStr );
    }

    if( bChange || bForce )
    {
        if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
        {
            Con_Printf( "CVoiceStatus::UpdateServerState: Sending '%s'\n", str );
        }

        gEngfuncs.pfnServerCmdUnreliable( str ); // Tell the server..
    }
    else
    {
        if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
        {
            gEngfuncs.pfnConsolePrint( "CVoiceStatus::UpdateServerState: no change\n" );
        }
    }

    m_LastUpdateServerState = gEngfuncs.GetClientTime();
}

void CVoiceStatus::UpdateSpeakerImage( Label* pLabel, int iPlayer )
{
    m_pBanButtons[iPlayer - 1] = pLabel;
    UpdateBanButton( iPlayer - 1 );
}

void CVoiceStatus::UpdateBanButton( int iClient )
{
    Label* pPanel = m_pBanButtons[iClient];

    if( !pPanel )
        return;

    char playerID[16];
    bool HACK_GetPlayerUniqueID( int iPlayer, char playerID[16] );
    if( !HACK_GetPlayerUniqueID( iClient + 1, playerID ) )
        return;

    // Figure out if it's blinking or not.
    bool bBlink = fmod( m_BlinkTimer, SCOREBOARD_BLINK_FREQUENCY * 2 ) < SCOREBOARD_BLINK_FREQUENCY;
    bool bTalking = !!m_VoicePlayers[iClient];
    bool bBanned = m_BanMgr.GetPlayerBan( playerID );
    bool bNeverSpoken = !m_VoiceEnabledPlayers[iClient];

    // Get the appropriate image to display on the panel.
    if( bBanned )
    {
        pPanel->setImage( m_pScoreboardBanned );
    }
    else if( bTalking )
    {
        if( bBlink )
        {
            pPanel->setImage( m_pScoreboardSpeaking2 );
        }
        else
        {
            pPanel->setImage( m_pScoreboardSpeaking );
        }
        pPanel->setFgColor( 255, 170, 0, 1 );
    }
    else if( bNeverSpoken )
    {
        pPanel->setImage( m_pScoreboardNeverSpoken );
        pPanel->setFgColor( 100, 100, 100, 1 );
    }
    else
    {
        pPanel->setImage( m_pScoreboardNotSpeaking );
    }
}


void CVoiceStatus::HandleVoiceMaskMsg( BufferReader& reader )
{
    unsigned long dw;
    for( dw = 0; dw < VOICE_MAX_PLAYERS_DW; dw++ )
    {
        m_AudiblePlayers.SetDWord( dw, (unsigned long)reader.ReadLong() );
        m_ServerBannedPlayers.SetDWord( dw, (unsigned long)reader.ReadLong() );

        if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
        {
            char str[256];
            gEngfuncs.pfnConsolePrint( "CVoiceStatus::HandleVoiceMaskMsg\n" );

            sprintf( str, "    - m_AudiblePlayers[%lu] = %u\n", dw, m_AudiblePlayers.GetDWord( dw ) );
            gEngfuncs.pfnConsolePrint( str );

            sprintf( str, "    - m_ServerBannedPlayers[%lu] = %u\n", dw, m_ServerBannedPlayers.GetDWord( dw ) );
            gEngfuncs.pfnConsolePrint( str );
        }
    }

    m_bServerModEnable = reader.ReadByte();
}

void CVoiceStatus::HandleReqStateMsg( BufferReader& reader )
{
    if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
    {
        gEngfuncs.pfnConsolePrint( "CVoiceStatus::HandleReqStateMsg\n" );
    }

    UpdateServerState( true );
}

void CVoiceStatus::StartSquelchMode()
{
    if( m_bInSquelchMode )
        return;

    m_bInSquelchMode = true;
    m_pHelper->UpdateCursorState();
}

void CVoiceStatus::StopSquelchMode()
{
    m_bInSquelchMode = false;
    m_pHelper->UpdateCursorState();
}

bool CVoiceStatus::IsInSquelchMode()
{
    return m_bInSquelchMode;
}

CVoiceLabel* CVoiceStatus::FindVoiceLabel( int clientindex )
{
    for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
    {
        if( m_Labels[i].m_clientindex == clientindex )
            return &m_Labels[i];
    }

    return nullptr;
}


CVoiceLabel* CVoiceStatus::GetFreeVoiceLabel()
{
    return FindVoiceLabel( -1 );
}


void CVoiceStatus::RepositionLabels()
{
    // find starting position to draw from, along right-hand side of screen
    int y = ScreenHeight / 2;

    int iconWide = 8, iconTall = 8;
    if( m_pSpeakerLabelIcon )
    {
        m_pSpeakerLabelIcon->getSize( iconWide, iconTall );
    }

    // Reposition active labels.
    for( int i = 0; i < MAX_VOICE_SPEAKERS; i++ )
    {
        CVoiceLabel* pLabel = &m_Labels[i];

        if( pLabel->m_clientindex == -1 || !pLabel->m_pLabel )
        {
            if( pLabel->m_pBackground )
                pLabel->m_pBackground->setVisible( false );

            continue;
        }

        int textWide, textTall;
        pLabel->m_pLabel->getContentSize( textWide, textTall );

        // Don't let it stretch too far across their screen.
        if( textWide > ( ScreenWidth * 2 ) / 3 )
            textWide = ( ScreenWidth * 2 ) / 3;

        // Setup the background label to fit everything in.
        int border = 2;
        int bgWide = textWide + iconWide + border * 3;
        int bgTall = std::max( textTall, iconTall ) + border * 2;
        pLabel->m_pBackground->setBounds( ScreenWidth - bgWide - 8, y, bgWide, bgTall );

        // Put the text at the left.
        pLabel->m_pLabel->setBounds( border, ( bgTall - textTall ) / 2, textWide, textTall );

        // Put the icon at the right.
        int iconLeft = border + textWide + border;
        int iconTop = ( bgTall - iconTall ) / 2;
        if( pLabel->m_pIcon )
        {
            pLabel->m_pIcon->setImage( m_pSpeakerLabelIcon );
            pLabel->m_pIcon->setBounds( iconLeft, iconTop, iconWide, iconTall );
        }

        y += bgTall + 2;
    }

    if( m_pLocalBitmap && m_pAckBitmap && m_pLocalLabel && ( m_bTalking || m_bServerAcked ) )
    {
        m_pLocalLabel->setParent( *m_pParentPanel );
        m_pLocalLabel->setVisible( true );

        if( m_bServerAcked && 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
            m_pLocalLabel->setImage( m_pAckBitmap );
        else
            m_pLocalLabel->setImage( m_pLocalBitmap );

        int sizeX, sizeY;
        m_pLocalBitmap->getSize( sizeX, sizeY );

        int local_xPos = ScreenWidth - sizeX - 10;
        int local_yPos = m_pHelper->GetAckIconHeight() - sizeY;

        m_pLocalLabel->setPos( local_xPos, local_yPos );
    }
    else
    {
        m_pLocalLabel->setVisible( false );
    }
}


void CVoiceStatus::FreeBitmaps()
{
    // Delete all the images we have loaded.
    delete m_pLocalBitmap;
    m_pLocalBitmap = nullptr;

    delete m_pAckBitmap;
    m_pAckBitmap = nullptr;

    delete m_pSpeakerLabelIcon;
    m_pSpeakerLabelIcon = nullptr;

    delete m_pScoreboardNeverSpoken;
    m_pScoreboardNeverSpoken = nullptr;

    delete m_pScoreboardNotSpeaking;
    m_pScoreboardNotSpeaking = nullptr;

    delete m_pScoreboardSpeaking;
    m_pScoreboardSpeaking = nullptr;

    delete m_pScoreboardSpeaking2;
    m_pScoreboardSpeaking2 = nullptr;

    delete m_pScoreboardSquelch;
    m_pScoreboardSquelch = nullptr;

    delete m_pScoreboardBanned;
    m_pScoreboardBanned = nullptr;

    // Clear references to the images in panels.
    for( int i = 0; i < MAX_PLAYERS; i++ )
    {
        if( m_pBanButtons[i] )
        {
            m_pBanButtons[i]->setImage( nullptr );
        }
    }

    if( m_pLocalLabel )
        m_pLocalLabel->setImage( nullptr );
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the target client has been banned
// Input  : playerID -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerBlocked( int iPlayer )
{
    char playerID[16];
    if( 0 == gEngfuncs.GetPlayerUniqueID( iPlayer, playerID ) )
        return false;

    return m_BanMgr.GetPlayerBan( playerID );
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the player can't hear the other client due to game rules (eg. the other team)
// Input  : playerID -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerAudible( int iPlayer )
{
    return !!m_AudiblePlayers[iPlayer - 1];
}

//-----------------------------------------------------------------------------
// Purpose: blocks/unblocks the target client from being heard
// Input  : playerID -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CVoiceStatus::SetPlayerBlockedState( int iPlayer, bool blocked )
{
    if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
    {
        gEngfuncs.pfnConsolePrint( "CVoiceStatus::SetPlayerBlockedState part 1\n" );
    }

    char playerID[16];
    if( 0 == gEngfuncs.GetPlayerUniqueID( iPlayer, playerID ) )
        return;

    if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
    {
        gEngfuncs.pfnConsolePrint( "CVoiceStatus::SetPlayerBlockedState part 2\n" );
    }

    // Squelch or (try to) unsquelch this player.
    if( 0 != gEngfuncs.pfnGetCvarFloat( "voice_clientdebug" ) )
    {
        char str[256];
        sprintf( str, "CVoiceStatus::SetPlayerBlockedState: setting player %d ban to %d\n", iPlayer, static_cast<int>( !m_BanMgr.GetPlayerBan( playerID ) ) );
        gEngfuncs.pfnConsolePrint( str );
    }

    m_BanMgr.SetPlayerBan( playerID, blocked );
    UpdateServerState( false );
}
