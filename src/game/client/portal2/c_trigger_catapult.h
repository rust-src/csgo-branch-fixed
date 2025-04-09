//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// copied from portal2 code; original code came with client-predicted counterpart,
// but implementing predictable triggers in tf2 wasn't trivial so this is just the
// server component. it works but causes prediction errors.
#ifndef C_TRIGGER_CATAPULT_H
#define C_TRIGGER_CATAPULT_H
#ifdef _WIN32
#pragma once
#endif

#include "c_triggers.h"
#define CTriggerCatapult C_TriggerCatapult

DECLARE_AUTO_LIST(ITriggerCatapultAutoList)

class C_TriggerCatapult : public C_BaseTrigger, public ITriggerCatapultAutoList
{
	DECLARE_CLASS(C_TriggerCatapult, C_BaseTrigger);
	//DECLARE_DATADESC();
	DECLARE_CLIENTCLASS();

public:

	C_TriggerCatapult(void);

	//virtual void	Spawn(void);
	virtual void	StartTouch(CBaseEntity* pOther);
	//virtual void	EndTouch(CBaseEntity* pOther);
	//virtual int		DrawDebugTextOverlays(void);
	//virtual void	DrawDebugGeometryOverlays(void);

	void			LaunchThink(void);

	C_BaseEntity* GetEntity() { return this; }

	void			PlayerPassesTriggerFiltersThink(void);
	static const char* s_szPlayerPassesTriggerFiltersThinkContext;

private:
	void			InputSetPlayerSpeed(inputdata_t& in);
	void			InputSetPhysicsSpeed(inputdata_t& in);
	void			InputSetLaunchTarget(inputdata_t& in);
	void			InputSetExactVelocityChoiceType(inputdata_t& in);

	void			LaunchByTarget(CBaseEntity* pVictim, CBaseEntity* pTarget);
	Vector			CalculateLaunchVector(CBaseEntity* pVictim, CBaseEntity* pTarget);
	Vector			CalculateLaunchVectorPreserve(Vector vecInitialVelocity, CBaseEntity* pVictim, CBaseEntity* pTarget, bool bForcePlayer = false);

	void			LaunchByDirection(CBaseEntity* pVictim);
	void			OnLaunchedVictim(CBaseEntity* pVictim);

	float m_flRefireDelay[MAX_PLAYERS + 1];
	float m_flPlayerVelocity;
	float m_flPhysicsVelocity;
	QAngle m_vecLaunchAngles;
	string_t m_strLaunchTarget;
	int m_ExactVelocityChoice;
	bool m_bUseExactVelocity;
	bool m_bUseThresholdCheck;
	float m_flLowerThreshold;
	float m_flUpperThreshold;
	float m_flEntryAngleTolerance;
	EHANDLE m_hLaunchTarget;
	bool m_bOnlyVelocityCheck;
	bool m_bApplyAngularImpulse;
	bool m_bPlayersPassTriggerFilters;
	float m_flAirControlSupressionTime;
	bool m_bDirectionSuppressAirControl;
};

#endif // C_TRIGGER_CATAPULT_H  
