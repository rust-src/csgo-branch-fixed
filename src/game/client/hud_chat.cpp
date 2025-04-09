//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hud_chat.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include "c_playerresource.h"
#include "hud_basechat.h"
#include "engine/ienginesound.h"
#include <vgui/ILocalize.h>

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


DECLARE_HUDELEMENT_FLAGS( CHudChat, HUDELEMENT_SS_FULLSCREEN_ONLY );

DECLARE_HUD_MESSAGE( CHudChat, SayText );
DECLARE_HUD_MESSAGE( CHudChat, SayText2 );
DECLARE_HUD_MESSAGE( CHudChat, TextMsg );

//=====================
//CHudChat
//=====================

CHudChat::CHudChat( const char *pElementName ) : BaseClass( pElementName )
{
	
}

void CHudChat::Init( void )
{
	BaseClass::Init();

	HOOK_HUD_MESSAGE( CHudChat, SayText );
	HOOK_HUD_MESSAGE( CHudChat, SayText2 );
	HOOK_HUD_MESSAGE( CHudChat, TextMsg );
}

//-----------------------------------------------------------------------------
// Purpose: Reads in a player's Chat text from the server
//-----------------------------------------------------------------------------
bool CHudChat::MsgFunc_SayText2( const CUsrMsg_SayText2 &msg )
{
	return BaseClass::MsgFunc_SayText2(msg);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
bool CHudChat::MsgFunc_SayText(const CUsrMsg_SayText& msg)
{
	return BaseClass::MsgFunc_SayText(msg);
}


// Message handler for text messages
// displays a string, looking them up from the titles.txt file, which can be localised
// parameters:
//   byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
//   string: message 
// optional parameters:
//   string: message parameter 1
//   string: message parameter 2
//   string: message parameter 3
//   string: message parameter 4
// any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
// the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
bool CHudChat::MsgFunc_TextMsg( const CUsrMsg_TextMsg &msg )
{
	return BaseClass::MsgFunc_TextMsg(msg);
}

