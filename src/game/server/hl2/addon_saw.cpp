#include "cbase.h"
//TODO

#include "ai_addon.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CAddOnSaw : public CAI_AddOn
{
public:
	DECLARE_CLASS(CAddOnSaw, CAI_AddOn);
	DECLARE_DATADESC();
	void Precache();
	char* GetAddOnModelName();
	Vector GetAttachOffset(QAngle& attachmentAngles);
	void UpdateOnRemove();
	float GetThinkInterval();
	bool Install(CAI_BaseNPC* pHost, bool bRemoveOnFail);
	void Remove();
	float m_flTargetSpeed;
	float m_flCurrentSpeed;
	CSoundPatch* m_pEngineSound;

//---------------------------------
// Schedule/Task/Conditions
//---------------------------------
	enum
	{
		TASK_START_SAW = NEXT_TASK,
		TASK_STOP_SAW,
		TASK_SPIN_SAW,
		NEXT_TASK,
	};

	enum
	{
		COND_HOST_ENEMY_NEAR = NEXT_CONDITION,
		COND_HOST_ENEMY_NOT_NEAR,
		NEXT_CONDITION,
	};

	enum
	{
		SCHED_START_SAW = NEXT_SCHEDULE,
		SCHED_STOP_SAW,
		NEXT_SCHEDULE,
	};

	DEFINE_AGENT();
};

BEGIN_DATADESC(CAddOnSaw)
DEFINE_FIELD(m_flTargetSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flCurrentSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_pEngineSound, FIELD_CUSTOM),
END_DATADESC()

//LINK_ENTITY_TO_CLASS(ai_addon_saw,CAddOnSaw)

void CAddOnSaw::Precache()
{
	CAI_AddOn::Precache();
	PrecacheScriptSound("Addon_Saw.Run");
	PrecacheScriptSound("Addon_Saw.Start");
}

float CAddOnSaw::GetThinkInterval()
{
	return 0.05;
}

bool CAddOnSaw::Install(CAI_BaseNPC* pHost, bool bRemoveOnFail)
{
	bool uVar2 = BaseClass::Install(pHost,bRemoveOnFail);
	if (!uVar2) 
	{
		return false;
	}

	GetNPCHost()->CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
	CPASAttenuationFilter local_48;
	CSoundPatch *pvVar8 = controller.SoundCreate(local_48, entindex(), "Addon_Saw.Run");
	m_pEngineSound = pvVar8;
	if (pvVar8) 
	{
		//controller.Play(pvVar8, *controller, 100.0f);
	}
	return true;
}

void CAddOnSaw::Remove()
{
	BaseClass::Remove();
	if (m_pEngineSound) 
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pEngineSound);
		m_pEngineSound = NULL;
	}
}

char * CAddOnSaw::GetAddOnModelName()
{
	return "models/props_junk/sawblade001a.mdl";
}

void CAddOnSaw::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	Remove();//Weird
}

AI_BEGIN_AGENT_(CAddOnSaw, CAI_AddOn)

DECLARE_CONDITION(COND_HOST_ENEMY_NEAR)
DECLARE_CONDITION(COND_HOST_ENEMY_NOT_NEAR)

DECLARE_TASK(TASK_START_SAW)
DECLARE_TASK(TASK_STOP_SAW)
DECLARE_TASK(TASK_SPIN_SAW)

DEFINE_SCHEDULE
(
	SCHED_START_SAW,
	"	Tasks"
	"	TASK_START_SAW		0"
	"	TASK_SPIN_SAW		0"
	"	"
	"	Interrupts"
	"	COND_HOST_ENEMY_NOT_NEAR"

)

DEFINE_SCHEDULE
(
	SCHED_STOP_SAW,
	"	Tasks"
	"	TASK_STOP_SAW	0"
	"	TASK_SPIN_SAW	0"
	"	"
	"	Interrupts"
	"	COND_HOST_ENEMY_NEAR"

)

AI_END_AGENT()