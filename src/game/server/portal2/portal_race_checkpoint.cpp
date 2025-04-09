#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char* g_szCheckpointStartSpinningThinkContext = "PortalRaceCheckpointStartSpinningThinkContext";

ConVar sv_portal_race_checkpoint_model_scale("sv_portal_race_checkpoint_model_scale", "2.0f", FCVAR_CHEAT);
ConVar sv_debug_portal_race_checkpoint("sv_debug_portal_race_checkpoint", "0", FCVAR_NONE);

color32 checkpointActiveColor = { 255, 106, 0, 255 };
color32 checkpointInactiveColor = { 0, 165, 255, 255 };

class CPortalRaceCheckpoint : public CBaseAnimating {

public:
	DECLARE_CLASS(CPortalRaceCheckpoint, CBaseAnimating);
	DECLARE_DATADESC();

	CNetworkVar(bool,m_bCheckpointActive);
	float m_fResetTime;
	bool m_bLocked;
	float m_fActivatedTime;
	float m_fLastTimerSound;
	COutputEvent m_OnCheckpointActivated;
	COutputEvent m_OnCheckpointReset;

	void InputActivate(inputdata_t&);
	void InputCancelActivate(inputdata_t&);
	void InputLock(inputdata_t&);
	void InputUnlock(inputdata_t&);
	void ActivatedThink();
	void StartTouch(CBaseEntity*);
	void StartSpinningThink();
	void OnCheckpointActivated();
	void OnCheckpointReset(bool);
	void Precache();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(portal_race_checkpoint, CPortalRaceCheckpoint)

BEGIN_DATADESC(CPortalRaceCheckpoint)
DEFINE_FIELD(m_bCheckpointActive, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_fResetTime, FIELD_FLOAT, "ResetTime"),
DEFINE_OUTPUT(m_OnCheckpointActivated, "OnCheckpointActivated"),
DEFINE_OUTPUT(m_OnCheckpointReset, "OnCheckpointReset"),
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "CancelActivate", InputCancelActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock),
DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock),
DEFINE_THINKFUNC(ActivatedThink),
DEFINE_THINKFUNC(StartSpinningThink),
END_DATADESC()

void CPortalRaceCheckpoint::ActivatedThink()
{
	float timeActivated = gpGlobals->curtime - m_fActivatedTime;
	if (m_fResetTime <= timeActivated && timeActivated != m_fResetTime)
	{
		OnCheckpointReset(true);
		return;
	}
	if (1.0 < gpGlobals->curtime - m_fLastTimerSound) 
	{
		EmitSound("Portal.room1_TickTock");
		m_fLastTimerSound = gpGlobals->curtime;
	}
	SetNextThink(gpGlobals->curtime + 0.1);
}

void CPortalRaceCheckpoint::InputActivate(inputdata_t &param_1)
{
	SetThink(&CPortalRaceCheckpoint::ActivatedThink);
	SetNextThink(gpGlobals->curtime + 0.1);
	m_fActivatedTime = gpGlobals->curtime;
	OnCheckpointActivated();
}

void CPortalRaceCheckpoint::InputCancelActivate(inputdata_t &param_1)
{
	m_bCheckpointActive = false;
	SetThink( NULL );
}

void CPortalRaceCheckpoint::InputLock(inputdata_t &param_1)
{
	SetLocalAngularVelocity( vec3_angle );
	m_bLocked = true;
}

void CPortalRaceCheckpoint::InputUnlock(inputdata_t &param_1)
{
	m_bLocked = false;
}

void CPortalRaceCheckpoint::OnCheckpointActivated()
{
	m_bCheckpointActive = true;
	EmitSound("Portal.button_down");
	m_fLastTimerSound = gpGlobals->curtime - 1.0;
	SetRenderColor(255, 255, 255);
	SetRenderColor(checkpointActiveColor.r,checkpointActiveColor.g,checkpointActiveColor.b);
	m_OnCheckpointActivated.FireOutput(this,this);
}

void CPortalRaceCheckpoint::Precache()
{
	BaseClass::Precache();
	PrecacheModel("models/effects/cappoint_hologram.mdl");
	PrecacheScriptSound("Portal.button_down");
	PrecacheScriptSound("Portal.button_up");
	PrecacheScriptSound("Portal.button_locked");
	PrecacheScriptSound("Portal.room1_TickTock");
}

void CPortalRaceCheckpoint::OnCheckpointReset(bool doOutput)
{
	m_bCheckpointActive = false;
	SetThink(NULL);
	if (doOutput) 
	{
		EmitSound("Portal.button_up");
		m_OnCheckpointReset.FireOutput(this, this);
		SetRenderColor(checkpointInactiveColor.r, checkpointInactiveColor.g, checkpointInactiveColor.b);
	}
}

void CPortalRaceCheckpoint::Spawn()
{

	Precache();

	SetModel("models/effects/cappoint_hologram.mdl");

	SetMoveType( MOVETYPE_NOCLIP );
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetCollisionGroup( COLLISION_GROUP_PLAYER );
	SetRenderColor( checkpointInactiveColor.r, checkpointInactiveColor.g, checkpointInactiveColor.b );

	SetModelScale(2.0);
	Vector newOrigin = GetAbsOrigin() + Vector(0, 0, -(sv_portal_race_checkpoint_model_scale.GetFloat() * 24.0));
	SetAbsOrigin(newOrigin);

	Vector vecMaxes(50,50,100);
	Vector vecMins(-50, -50, 0);
	SetSize(vecMins, vecMaxes);

	SetContextThink(&CPortalRaceCheckpoint::StartSpinningThink, RandomFloat(0, 0.5) + gpGlobals->curtime, g_szCheckpointStartSpinningThinkContext);
	
	BaseClass::Spawn();
}

void CPortalRaceCheckpoint::StartSpinningThink()
{
	ApplyLocalAngularVelocityImpulse( AngularImpulse(0,0,180) );
}

void CPortalRaceCheckpoint::StartTouch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer()) 
	{

		if (m_bLocked) 
		{
			EmitSound("Portal.button_locked");
		}
		else 
		{
			SetThink(&CPortalRaceCheckpoint::ActivatedThink);
			SetNextThink(gpGlobals->curtime + 0.1);
			m_fActivatedTime = gpGlobals->curtime;
			OnCheckpointActivated();
		}
	}
}