#include "cbase.h"
#include "props.h"
#include <particle_parse.h>

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"



ConVar futbol_shooter_distance_from_target("futbol_shooter_distance_from_target", "64", 0x4000);
ConVar futbol_shooter_target_reticule_enabled("futbol_shooter_target_reticule_enabled", "0", 0x4000);
ConVar futbol_shooter_target_height_offset("futbol_shooter_target_height_offset", "0", 0x4000);
extern ConVar sv_gravity;

class CPointFutbolShooter : public CPointEntity
{
public:
	DECLARE_CLASS(CPointFutbolShooter,CPointEntity);
	DECLARE_DATADESC();
	void Precache();
	void Spawn();

	void InputSetTarget(inputdata_t &);
	void InputShootFutbol(inputdata_t&);


	float m_flPhysicsSpeed;
	COutputEvent m_OutputShootSuccess;
	COutputEvent m_OutputShootFail;
    Vector m_vTarget;
};

BEGIN_DATADESC(CPointFutbolShooter)
DEFINE_KEYFIELD(m_flPhysicsSpeed,FIELD_FLOAT,"LaunchSpeed"),
DEFINE_INPUTFUNC(FIELD_STRING,"SetTarget",InputSetTarget),
DEFINE_INPUTFUNC(FIELD_STRING, "ShootFutbol", InputShootFutbol),
DEFINE_OUTPUT(m_OutputShootSuccess,"OnShootSuccess"),
DEFINE_OUTPUT(m_OutputShootFail, "OnShootFail"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(point_futbol_shooter,CPointFutbolShooter)

void CPointFutbolShooter::Precache()
{
	UTIL_PrecacheOther("prop_exploding_futbol");
	PrecacheParticleSystem("command_target_ping");
	PrecacheScriptSound("World.Wheatley.fire");
}

void CPointFutbolShooter::Spawn()
{
	BaseClass::Spawn();
	Precache();
}


void CPointFutbolShooter::InputSetTarget(inputdata_t &param_1)
{

    const char * pvVar3 = param_1.value.String();
    CBaseEntity * pCVar4 = gEntList.FindEntityByName(NULL, pvVar3);
    if (pCVar4) 
    {
        m_vTarget = pCVar4->GetAbsOrigin();
    }

    DevMsg("Invalid target entity!\n");
}

void CPointFutbolShooter::InputShootFutbol(inputdata_t &param_1)

{
    float fVar1;
    IPhysicsObject* pIVar2;
    int iVar3;
    Vector VVar4;
    QAngle QVar5;
    CBaseEntity* this_00;
    CPhysicsProp* pvVar7;
    CBasePlayer* pCVar6;
    //float10 fVar7;
    float fVar8;
    //undefined4 uVar9;
    Vector local_54;
    Vector local_48;
    Vector local_3c;
    Vector local_30;
    Vector local_24;
    //undefined4 local_14;

    local_48 = GetAbsOrigin();
    local_30 = m_vTarget;
    local_24 = local_30 - local_48;

    if (0.0 < futbol_shooter_distance_from_target.GetFloat()) {
        local_3c.y = local_24.y;
        local_3c.x = local_24.x;
        local_3c.z = 0.0;
        VectorNormalize(local_3c);
        fVar1 = futbol_shooter_distance_from_target.GetFloat();
        local_30.z = local_30.z - fVar1 * local_3c.z;
        local_30.y = local_30.y - local_3c.y * fVar1;
        local_30.x = local_30.x - local_3c.x * fVar1;
    }

    AngleVectors(GetAbsAngles(), &local_54, NULL,NULL);
    if (local_54.y * local_24.y + local_24.x * local_54.x < 0.0) 
    {
        m_OutputShootFail.FireOutput(param_1.pActivator, param_1.pCaller);
        return;
    }

    this_00 = CreateEntityByName("prop_exploding_futbol", -1, true);
    if (this_00 == NULL) 
    {
        DevMsg("Failed to create exploding futbol\n!");
        return;
    }

    this_00->SetAbsOrigin(local_48);
    this_00->Spawn();//Isn't dispatchSpawn meant to be called???
    //(**(code**)(this_00->_padding_ + 0x60))();
    fVar8 = sqrt(local_24.y * local_24.y + local_24.z * local_24.z + local_24.x * local_24.x) / m_flPhysicsSpeed;
    fVar1 = 1.0 / fVar8;
    local_24.x = local_24.x * fVar1;
    local_24.y = local_24.y * fVar1;
    local_24.z = fVar8 * sv_gravity.GetFloat() * 0.5 + fVar1 * local_24.z;

    pIVar2 = this_00->VPhysicsGetObject();
    if (pIVar2) 
    {
        float fVar7 = RandomFloat(-150, 150);
        local_3c.x = (float)fVar7;
        fVar7 = RandomFloat(-150, 150);
        local_3c.y = (float)fVar7;
        fVar7 = RandomFloat(-150, 150);
        local_3c.z = (float)fVar7;
        pIVar2->SetVelocityInstantaneous(&local_24, &local_3c);
        float local_14 = 0;
        pIVar2->SetDragCoefficient(&local_14, &local_14);
        pIVar2->SetDamping(&local_14, &local_14);
        pvVar7 = dynamic_cast<CPhysicsProp *>(this_00);
        if (pvVar7) 
        {
            pvVar7->OnPhysGunDrop(UTIL_GetLocalPlayer(), LAUNCHED_BY_CANNON);
        }
    }
    m_OutputShootSuccess.FireOutput(param_1.pActivator, param_1.pCaller);
    EmitSound("World.Wheatley.fire");

    if (futbol_shooter_target_reticule_enabled.GetBool()) 
    {
        local_3c.z = 0.0;
        local_3c.x = 255.0;
        local_3c.y = 0.0;
        VVar4.z = local_30.z - futbol_shooter_target_height_offset.GetFloat();
        VVar4.x = local_30.x;
        VVar4.y = local_30.y;
        QVar5 = vec3_angle;
        //DispatchParticleEffect("command_target_ping", VVar4, (Vector)ZEXT812(0x437f0000), QVar5);
    }
}
