//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Aperture Incentivising Nugget (item_nugget)
//
//=============================================================================//

#include "cbase.h"
#include "portal_player.h"
#include "team.h"
#include "UtlSortVector.h"
#include "portal_usermessages.pb.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItem_Nugget : public CBaseAnimating
{
public:
    DECLARE_CLASS(CItem_Nugget, CBaseAnimating);
    DECLARE_DATADESC();
    void Spawn();
    void Precache();
    CBaseEntity* Respawn();
    void MaterializeThink();
    void UpdateOnRemove();
    void StartTouch(CBaseEntity*);
    CNetworkVar(int, m_nValue);
    float m_fRespawnTime;
    string_t m_iGroupName;
};
ConVar sv_randomize_nugget_availability("sv_randomize_nugget_availability", "0", 0, "[PORTALMP] Randomize which nuggets are available on map start");
ConVar sv_randomize_nugget_availability_groupavailability("sv_randomize_nugget_availability_groupavailability", "0.5", 0, "[PORTALMP] 0.0 to 1.0 chances that a group of nuggets exists after randomization");
ConVar sv_randomize_nugget_availability_ungroupedavailability("sv_randomize_nugget_availability_ungroupedavailability", "0.5", 0, "[PORTALMP] 0.0 to 1.0 chances that an individual ungrouped nugget exists after randomization");

class CNuggetDirector : public CAutoGameSystem
{
public:
    class CNuggetGroupSort
    {
    public:
        bool Less(const CItem_Nugget* src1, const CItem_Nugget* src2, void* pCtx)
        {
            //if (stricmp(src1->GetClassname(), src2->GetClassname()) < 0)
            //    return true;

            return false;
        }
    };
    void LevelInitPostEntity();
    void RandomizeAvailableNuggets(float, float);
    void RegisterNugget(CItem_Nugget *);
    void UnregisterNugget(CItem_Nugget* param_1);
    CUtlVector<CItem_Nugget*> m_UngroupedNuggets;

    CUtlSortVector<CItem_Nugget*, CNuggetGroupSort> m_GroupedNuggets;
};

CNuggetDirector g_NuggetDirector;

void CNuggetDirector::LevelInitPostEntity()
{
    if (sv_randomize_nugget_availability.GetBool()) 
    {
        RandomizeAvailableNuggets(sv_randomize_nugget_availability_ungroupedavailability.GetFloat(), sv_randomize_nugget_availability_groupavailability.GetFloat());
    }
}

void CNuggetDirector::RandomizeAvailableNuggets(float, float)
{
    DevMsg("Add later. This is not that important");
}

void CNuggetDirector::RegisterNugget(CItem_Nugget* param_1)
{
    //if (STRING(param_1->m_iGroupName) != "") ???
    {
        if (V_stricmp(STRING(param_1->m_iGroupName), "") != 0) {
            m_GroupedNuggets.Insert(param_1);
            return;
        }
    }

    m_UngroupedNuggets.AddToTail(param_1);//Check if correct
}

void CNuggetDirector::UnregisterNugget(CItem_Nugget* param_1)
{

    //if (STRING(param_1->m_iGroupName) != "") ???
    {
        if (V_stricmp(STRING(param_1->m_iGroupName), "") != 0) {
            m_GroupedNuggets.Remove(param_1);
            return;
        }
    }
    m_UngroupedNuggets.FindAndFastRemove(param_1);
}

BEGIN_DATADESC(CItem_Nugget)
DEFINE_KEYFIELD(m_iGroupName,FIELD_STRING,"GroupName"),
DEFINE_KEYFIELD(m_fRespawnTime,FIELD_FLOAT,"RespawnTime"),
DEFINE_KEYFIELD(m_nValue,FIELD_INTEGER,"PointValue"),
DEFINE_THINKFUNC(MaterializeThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(item_nugget,CItem_Nugget)

void CItem_Nugget::StartTouch(CBaseEntity* param_1)
{
    if (((GetEffects() & EF_NODRAW) == 0) && param_1->IsPlayer()) 
    {
        CBasePlayer* param_ = ToBasePlayer(param_1);
        CSingleUserRecipientFilter local_24(param_);

        CUsrMsg_ItemPickup msg;
        msg.set_item(STRING(m_iClassname));
        SendUserMessage(local_24, UM_ItemPickup, msg);

        CPASAttenuationFilter local_44(param_1, "Nugget.Touch");
        CBaseEntity::EmitSound(local_44, param_1->entindex(), "Nugget.Touch");
        param_->IncrementFragCount(m_nValue);
        if (param_1->GetTeam()) 
        {
            param_1->GetTeam()->AddScore(m_nValue);
        }
        if (m_fRespawnTime <= 0.0) 
        {
            UTIL_Remove(this);
        }
        else 
        {
            Respawn();
        }
    }
}

void CItem_Nugget::Precache()
{
    PrecacheModel("models/effects/cappoint_hologram.mdl");
    PrecacheScriptSound("Nugget.Touch");
    PrecacheScriptSound("Nugget.Spawn");
}

CBaseEntity * CItem_Nugget::Respawn()
{
    AddEffects(EF_NODRAW);
    SetThink(&CItem_Nugget::MaterializeThink);
    CBaseEntity::SetNextThink(m_fRespawnTime + gpGlobals->curtime);
    return this;
}

void CItem_Nugget::UpdateOnRemove()
{
    g_NuggetDirector.UnregisterNugget(this);
    BaseClass::UpdateOnRemove();
}

void CItem_Nugget::MaterializeThink()
{
    if (GetEffects() & EF_NODRAW) {
        EmitSound("Nugget.Spawn");
        RemoveEffects(EF_NODRAW);
    }
}

void CItem_Nugget::Spawn()
{

	Precache();

	SetModel("models/effects/cappoint_hologram.mdl");

    float local_8 = 0.5;
    if (m_nValue < 5) 
    {
        m_nValue = 1;
        m_nSkin = 1;
    }
    else if (m_nValue < 10) 
    {
        m_nValue = 5;
        m_nSkin = 2;
        local_8 = 0.6;
    }
    else if (m_nValue < 25) 
    {
        m_nValue = 10;
        m_nSkin = 3;
        local_8 = 0.8;
    }
    else 
    {
        m_nValue = 25;
        m_nSkin = 4;
        local_8 = 1.0;
    }

	SetMoveType(MOVETYPE_NOCLIP);
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);

    SetModelScale(local_8);
    ApplyLocalAngularVelocityImpulse(AngularImpulse(0, 0, 180));

    local_8 = local_8 * 24.0;
	
	Vector newOrigin = GetAbsOrigin() + Vector(0, 0, -local_8);
	SetAbsOrigin(newOrigin);

	Vector vecMaxes;
	Vector vecMins;

    vecMaxes.x = local_8;
    vecMaxes.y = local_8;
    vecMaxes.z = local_8 - -local_8;
    vecMins.x = -local_8;
    vecMins.y = -local_8;
    vecMins.z = -local_8 - -local_8;

	SetSize(vecMins, vecMaxes);

    //CPortalPlayerInventory::ValidateInventoryPositions();
    g_NuggetDirector.RegisterNugget(this);
}

