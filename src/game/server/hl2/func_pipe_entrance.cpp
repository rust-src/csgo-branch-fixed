//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ====
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "func_pipe_entrance.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CFunc_Pipe_Entrance)
DEFINE_KEYFIELD( m_PipeSystemName, FIELD_STRING, "pipesystemname" ),
DEFINE_KEYFIELD( m_fFlowRate, FIELD_FLOAT, "flowrate" ),
DEFINE_KEYFIELD( m_fFlowTimeStamp, FIELD_FLOAT, "flowtimestamp" ),
DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC()

CUtlVector<CFunc_Pipe_Entrance*> m_PipeEntrances;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFunc_Pipe_Entrance::CFunc_Pipe_Entrance()
{
	int index = m_PipeEntrances.AddToTail(this);
    m_iIndexInArray = index;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Spawn()
{
	if (m_fFlowRate <= 0.0) {
		m_fFlowRate = 20.0;
	}
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::FUN_1800a7080()
{
    if (STRING(m_PipeSystemName) && (*STRING(m_PipeSystemName))) 
    {
        for (int i = m_PipeEntrances.Count() - 1; i > 0; i--)
        {
            CFunc_Pipe_Entrance * pCVar3 = m_PipeEntrances[i];
            if (pCVar3 == this)
            {
                CNodeEnt * pCVar4 = (CNodeEnt*)CreateEntityByName("info_node");
                pCVar4->SetAbsOrigin(GetAbsOrigin());
                pCVar4->m_NodeData.nWCNodeID = m_iIndexInArray + 10000;
                DispatchSpawn(pCVar4, true);
            }
            else if (pCVar3->m_PipeSystemName == m_PipeSystemName)
            {
                //There is so much inlined code here
                /*
                CAI_DynamicLink* pCStack_20;
                int iVar6 = (pCVar3->field16_0x550).m_Size + -1;
                if (-1 < iVar6) {
                    lVar7 = iVar6;
                    ppCVar3 = &((pCVar3->field16_0x550).m_pMemory)->vftablePtr + lVar7 * 2;
                    do {
                        if (*ppCVar3 == (CBaseEntity_vftable*)this) {
                            if (iVar6 != -1) {
                                pCStack_20 = *(CBaseAnimating**)
                                    (&(((pCVar3->field16_0x550).m_pMemory)->field1_0x8).field_0x0 +
                                        iVar6 * 0x10);
                                goto LAB_1800a7218;
                            }
                            break;
                        }
                        iVar6 = iVar6 + -1;
                        ppCVar3 = ppCVar3 + -2;
                        lVar7 = lVar7 + -1;
                    } while (-1 < lVar7);
                }

                pCStack_20 = (CAI_DynamicLink *)CreateEntityByName("info_node_link", -1, true);
                *(int*)&(pCStack_20->field1_0x8).field_0x538 = m_iIndexInArray + 10000;
                *(int*)&(pCStack_20->field1_0x8).field_0x53c = i + 10000;
                *(undefined4*)&pCStack_20[1].field1_0x8.field0_0x0.field_0x12 = 0x10;
                uVar8 = *(uint*)&(pCStack_20->field1_0x8).field0_0x0.field_0x474 | 0x800;
                if (*(uint*)&(pCStack_20->field1_0x8).field0_0x0.field_0x474 != uVar8) {
                    CBaseAnimating::vfunction198(pCStack_20, (short)pCStack_20 + 0x47c);
                    *(uint*)&(pCStack_20->field1_0x8).field0_0x0.field_0x474 = uVar8;
                }
                *(undefined4*)((longlong)&pCStack_20[1].vftablePtr + 6) = 1;

                DispatchSpawn((longlong*)pCStack_20, true);
            LAB_1800a7218:
                field16_0x550.
                iVar6 = (this->field16_0x550).m_Size;
                uVar8 = (this->field16_0x550).m_nAllocationCount;
                uVar9 = iVar6 + 1;
                if ((int)uVar8 < (int)uVar9) {
                    iVar2 = (this->field16_0x550).m_nGrowSize;
                    if (-1 < iVar2) {
                        if (iVar2 == 0) {
                            if (uVar8 == 0) {
                                uVar8 = 2;
                                if (2 < (int)uVar9) {
                                    uVar8 = uVar9;
                                }
                            }
                            else {
                                do {
                                    uVar8 = uVar8 * 2;
                                } while ((int)uVar8 < (int)uVar9);
                            }
                        }
                        else {
                            uVar8 = (iVar2 - iVar6 % iVar2) + iVar6;
                            if ((int)uVar8 < (int)uVar9) {
                                if ((uVar8 == 0) && (0x7fffffff < uVar9)) {
                                    uVar8 = 0xffffffff;
                                }
                                else {
                                    do {
                                        uVar8 = (int)(uVar8 + uVar9) / 2;
                                    } while ((int)uVar8 < (int)uVar9);
                                }
                            }
                        }
                        (this->field16_0x550).m_nAllocationCount = uVar8;
                        pCVar5 = (this->field16_0x550).m_pMemory;
                        lVar7 = **(longlong**)g_pMemAlloc_exref;
                        if (pCVar5 == (CBaseEntity*)0x0) {
                            pCVar5 = (CBaseEntity*)(**(code**)(lVar7 + 8))();
                        }
                        else {
                            pCVar5 = (CBaseEntity*)
                                (**(code**)(lVar7 + 0x18))
                                (*(longlong**)g_pMemAlloc_exref, pCVar5, (longlong)(int)uVar8 << 4);
                        }
                        (this->field16_0x550).m_pMemory = pCVar5;
                    }
                }
                piVar1 = &(this->field16_0x550).m_Size;
                *piVar1 = *piVar1 + 1;
                pCVar5 = (this->field16_0x550).m_pMemory;
                *(CBaseEntity**)((longlong) & (this->field16_0x550).m_pElements + 4) = pCVar5;
                ppCVar3 = &pCVar5->vftablePtr + (longlong)iVar6 * 2;
                *ppCVar3 = (CBaseEntity_vftable*)pCVar3;
                *(CBaseAnimating**)(ppCVar3 + 1) = pCStack_20;
                */
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputToggle(inputdata_t & inputdata)
{
	Toggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputEnable(inputdata_t& inputdata)
{
	Enable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputDisable(inputdata_t& inputdata)
{
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Enable()
{
	field_0x538 = false;
	IPhysicsObject * plVar2 = VPhysicsGetObject();
	if (plVar2) 
	{
		plVar2->EnableCollisions(true);
	}
	if (!IsSolidFlagSet(FSOLID_TRIGGER)) 
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}

    for (int i = field16_0x550.Count() - 1; -1 < i; i--)
    {
        if (!field16_0x550[i]->pPipe->field_0x538)
        {
            CAI_DynamicLink* plVar12 = field16_0x550[i]->pLink;
            plVar12->m_nLinkState = LINK_ON;
            plVar12->SetLinkState();
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFunc_Pipe_Entrance::~CFunc_Pipe_Entrance()
{
	m_PipeEntrances.FindAndRemove(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Disable()
{
	field_0x538 = true;
	IPhysicsObject * plVar1 = VPhysicsGetObject();
	if (plVar1) {
		plVar1->EnableCollisions(false);
	}
	if (IsSolidFlagSet(FSOLID_TRIGGER)) 
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}

    for (int i = field16_0x550.Count() - 1; -1 < i; i--)
    {
        CAI_DynamicLink* plVar12 = field16_0x550[i]->pLink;
        plVar12->m_nLinkState = LINK_OFF;
        plVar12->SetLinkState();
    }

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Toggle()
{
	if (IsSolidFlagSet(FSOLID_TRIGGER))
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else 
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}
	
	PhysicsTouchTriggers();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Activate()
{
	FUN_1800a7080();
	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_NONE, MOVECOLLIDE_DEFAULT);
	SetModel(GetModelName().ToCStr());
	AddEffects(EF_NODRAW);
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::UpdateOnRemove()
{
    //Weird why does it need to remove itself from its own array, then destroy the array?????
    //That is totally pointless
    /*
    longlong lVar1;
    int iVar2;
    int iVar3;
    int * plVar4;
    int iVar5;
    int lVar6;
    int lVar7;
    int lVar8;

    iVar2 = (this->field16_0x550).m_Size + -1;
    lVar7 = (int)iVar2;
    if (-1 < iVar2) {
        lVar8 = lVar7 * 16;
        do {
            lVar1 = *(int*)((int) & ((this->field16_0x550).m_pMemory)->pPipe + lVar8);
            iVar2 = lVar1.m_Size + -1;
            if (-1 < iVar2) {
                lVar6 = (int)iVar2;
                plVar4 = (int*)(lVar6 * 16 + *(int*)(lVar1 + 0x550));
                do {
                    if ((CFunc_Pipe_Entrance*)*plVar4 == this) goto LAB_1800a73dd;
                    iVar2 = iVar2 + -1;
                    plVar4 = plVar4 + -2;
                    lVar6 = lVar6 + -1;
                } while (-1 < lVar6);
            }
            iVar2 = -1;
        LAB_1800a73dd:
            iVar5 = *(int*)(lVar1 + 0x560);
            iVar3 = (iVar5 - iVar2) + -1;
            if (0 < iVar3) {
                memcpy((int*)(iVar2 * 0x10 + *(int*)(lVar1 + 0x550)),
                    (int*)((iVar2 + 1) * 0x10 + *(int*)(lVar1 + 0x550)),
                    (int)iVar3 * 16);
                iVar5 = *(int*)(lVar1 + 0x560);
            }
            lVar8 = lVar8 + -0x10;
            lVar7 = lVar7 + -1;
            *(int*)(lVar1 + 0x560) = iVar5 + -1;
        } while (-1 < lVar7);
    }*/

    field16_0x550.RemoveAll();

    //(this->field16_0x550).m_Size = 0;

    BaseClass::UpdateOnRemove();
}