//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "VGUI_Panel.h"
#include "VGUI_Label.h"


inline int PanelTop( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return y;
}
inline int PanelLeft( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return x;
}
inline int PanelRight( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return x + w;
}
inline int PanelBottom( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return y + h;
}
inline int PanelWidth( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return w;
}
inline int PanelHeight( vgui::Panel* pPanel )
{
    int x, y, w, h;
    pPanel->getBounds( x, y, w, h );
    return h;
}

// Places child at the requested position inside pParent. iAlignment is from Label::Alignment.
void AlignPanel( vgui::Panel* pChild, vgui::Panel* pParent, int alignment );
