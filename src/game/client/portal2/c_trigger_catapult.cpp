#include "cbase.h"

#include "c_trigger_catapult.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST(ITriggerCatapultAutoList)

#undef CTriggerCatapult

IMPLEMENT_CLIENTCLASS_DT(C_TriggerCatapult, DT_TriggerCatapult, CTriggerCatapult)
RecvPropArray3(RECVINFO_ARRAY(m_flRefireDelay), RecvPropFloat(RECVINFO_ARRAY(m_flRefireDelay))),
RecvPropFloat(RECVINFO(m_flPlayerVelocity)),
RecvPropFloat(RECVINFO(m_flPhysicsVelocity)),
RecvPropQAngles(RECVINFO(m_vecLaunchAngles)),
//RecvPropStringT( RECVINFO( m_strLaunchTarget ) ),
RecvPropInt(RECVINFO(m_ExactVelocityChoice)),
RecvPropBool(RECVINFO(m_bUseExactVelocity)),
RecvPropBool(RECVINFO(m_bUseThresholdCheck)),
RecvPropBool(RECVINFO(m_bOnlyVelocityCheck)),
RecvPropFloat(RECVINFO(m_flLowerThreshold)),
RecvPropFloat(RECVINFO(m_flUpperThreshold)),
RecvPropFloat(RECVINFO(m_flAirControlSupressionTime)),
RecvPropBool(RECVINFO(m_bApplyAngularImpulse)),
RecvPropFloat(RECVINFO(m_flEntryAngleTolerance)),
RecvPropEHandle(RECVINFO(m_hLaunchTarget)),
RecvPropBool(RECVINFO(m_bPlayersPassTriggerFilters)),
RecvPropBool(RECVINFO(m_bDirectionSuppressAirControl)),
END_RECV_TABLE()

C_TriggerCatapult::C_TriggerCatapult()
{

}