#include "cbase.h"
#include "ai_addon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class CAI_AddonBuilder : public CPointEntity
{
public:
	DECLARE_CLASS(CAI_AddonBuilder, CPointEntity);
	DECLARE_DATADESC();
	void InputDisable(inputdata_t &);
	void InputEnable(inputdata_t &);
	void InputExecute(inputdata_t &);
	void Execute();
	int DrawDebugTextOverlays();
	COutputEvent m_OnCreateNpc;
	COutputEvent m_OnFailedToCreateNpc;
	COutputEvent m_OnCreateAddon;
	COutputEvent m_OnFailedToCreateAddon;
	string_t m_iszClassnameOrTemplate;
	string_t m_iszNPCName;
	string_t m_iszAddOnName;
	int m_nNpcPoints;
	int m_nAddonPoints;
	int m_iCost;
	bool m_bDisabled;
	EHANDLE m_hBlinkEntity;
};

BEGIN_DATADESC(CAI_AddonBuilder)
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Execute", InputExecute),
DEFINE_OUTPUT(m_OnCreateNpc, "OnCreateNpc"),
DEFINE_OUTPUT(m_OnFailedToCreateNpc, "OnFailedToCreateNpc"),
DEFINE_OUTPUT(m_OnCreateAddon, "OnCreateAddon"),
DEFINE_OUTPUT(m_OnFailedToCreateAddon, "OnFailedToCreateAddon"),
DEFINE_FIELD(m_iszClassnameOrTemplate, FIELD_STRING),
DEFINE_KEYFIELD(m_iszNPCName, FIELD_STRING, "NPCName"),
DEFINE_KEYFIELD(m_iszAddOnName, FIELD_STRING, "AddOnName"),
DEFINE_KEYFIELD(m_nNpcPoints, FIELD_INTEGER, "NpcPoints"),
DEFINE_KEYFIELD(m_nAddonPoints, FIELD_INTEGER, "AddonPoints"),
DEFINE_FIELD(m_iCost, FIELD_INTEGER),
DEFINE_FIELD(m_hBlinkEntity, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(ai_addon_builder,CAI_AddonBuilder)

int CAI_AddonBuilder::DrawDebugTextOverlays()
{
	int iVar1 = CBaseEntity::DrawDebugTextOverlays();
	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		V_snprintf(tempstr, sizeof(tempstr), "NPC Points: %d", m_nNpcPoints);
		EntityText(iVar1, tempstr, 0);
		iVar1++;
		V_snprintf(tempstr, sizeof(tempstr), "Addon Points: %d", m_nAddonPoints);
		EntityText(iVar1, tempstr, 0);
	}
	return iVar1;
}

void CAI_AddonBuilder::InputEnable(inputdata_t &)
{
	m_bDisabled = false;
}

void CAI_AddonBuilder::InputDisable(inputdata_t&)
{
	m_bDisabled = true;
}

void CAI_AddonBuilder::InputExecute(inputdata_t&)
{
	if (!m_bDisabled)
	{
		Execute();
	}
}

void CAI_AddonBuilder::Execute() 
{
	//TODO
}