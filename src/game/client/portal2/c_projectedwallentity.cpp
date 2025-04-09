#include "cbase.h"
#include "c_projectedwallentity.h"
#include "c_world.h"
#include "mathlib/polyhedron.h"
#include "portal_player_shared.h"
#include "prediction.h"
#include "c_prop_portal.h"
#include "materialsystem/imaterialvar.h"
#include "c_portal_player.h"

#undef CProjectedWallEntity

#define PROJECTED_WALL_WIDTH 64.0f
#define PROJECTED_WALL_HEIGHT 0.015625 // 1/64 - thickness of the bridge

static int s_iActiveCollideableFlatFieldOffset = -1;

unsigned char g_nWhite[] =
{
	255,
	255,
	255,
	255
};

extern ConVar sv_thinnerprojectedwalls;
ConVar cl_draw_projected_wall_with_paint( "cl_draw_projected_wall_with_paint", "0", FCVAR_CLIENTDLL ); // 1 by default, but 0 for now
ConVar cl_projected_wall_projection_speed( "cl_projected_wall_projection_speed", "150", FCVAR_CLIENTDLL );

IMPLEMENT_AUTO_LIST( IProjectedWallEntityAutoList )

IMPLEMENT_CLIENTCLASS_DT( C_ProjectedWallEntity, DT_ProjectedWallEntity, CProjectedWallEntity )
	RecvPropFloat( RECVINFO( m_flLength ) ),
	RecvPropFloat( RECVINFO( m_flHeight ) ),
	RecvPropFloat( RECVINFO( m_flWidth ) ),
	RecvPropFloat( RECVINFO( m_flSegmentLength ) ),
	RecvPropFloat( RECVINFO( m_flParticleUpdateTime ) ),
	
	RecvPropBool( RECVINFO( m_bIsHorizontal ) ),
	RecvPropInt( RECVINFO( m_nNumSegments ) ),
	
	RecvPropVector( RECVINFO( m_vWorldSpace_WallMins ) ),
	RecvPropVector( RECVINFO( m_vWorldSpace_WallMaxs ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ProjectedWallEntity )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( projected_wall_entity, C_ProjectedWallEntity )

C_ProjectedWallEntity::C_ProjectedWallEntity()
{
	m_pBodyMaterial = NULL;
	m_pSideRailMaterial = NULL;
	m_pPaintMaterialsMid[0] = NULL;
	m_pPaintMaterialsEnd1[0] = NULL;
	m_pPaintMaterialsEnd2[0] = NULL;
	m_pPaintMaterialsSing[0] = NULL;
	m_pPaintMaterialsMid[1] = NULL;
	m_pPaintMaterialsEnd1[1] = NULL;
	m_pPaintMaterialsEnd2[1] = NULL;
	m_pPaintMaterialsSing[1] = NULL;
	m_pPaintMaterialsMid[2] = NULL;
	m_pPaintMaterialsEnd1[2] = NULL;
	m_pPaintMaterialsEnd2[2] = NULL;
	m_pPaintMaterialsSing[2] = NULL;
	m_pPaintMaterialsMid[3] = NULL;
	m_pPaintMaterialsEnd1[3] = NULL;
	m_pPaintMaterialsEnd2[3] = NULL;
	m_pPaintMaterialsSing[3] = NULL;
	m_pPaintMaterialRBounceLSpeed = NULL;
	m_pPaintMaterialLBounceRSpeed = NULL;
	m_pPaintMaterialBounceRSpeed = NULL;
	m_pPaintMaterialBounceLSpeed = NULL;
	m_pPaintMaterialBounceLRSpeed = NULL;
	m_nNumSegments = NULL;
	m_flCurDisplayLength = 0.0;
	m_flSegmentLength = 0.0;
	m_flParticleUpdateTime = 0.0;
	m_flPrevParticleUpdateTime = 0.0;
}

void C_ProjectedWallEntity::Spawn( void )
{
	SetThink( NULL );
	BaseClass::Spawn();
}

void C_ProjectedWallEntity::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		InitMaterials(); // Might be inaccurate
		SetupWallParticles();
	}

	if ( fabs( m_vLastStartpos.x - m_vecStartPoint.x ) > 0.1 ||
		fabs( m_vLastStartpos.y - m_vecStartPoint.y ) > 0.1 ||
		fabs( m_vLastStartpos.z - m_vecStartPoint.z ) > 0.1 )
	{
		m_flCurDisplayLength = 0.0;
		m_vLastStartpos = m_vecStartPoint;
	}

	CollisionProp()->MarkSurroundingBoundsDirty();
	CollisionProp()->MarkPartitionHandleDirty();
}

void C_ProjectedWallEntity::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );
	m_flPrevParticleUpdateTime = m_flParticleUpdateTime;
}

void C_ProjectedWallEntity::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );
}

void C_ProjectedWallEntity::OnProjected( void )
{
	BaseClass::OnProjected();
	ProjectWall();
	SetupWallParticles();
}

void C_ProjectedWallEntity::ProjectWall( void )
{
	AddEffects( EF_NOINTERP );
	//CheckForPlayersOnBridge();

	const Vector vStartPoint = GetStartPoint();
	const Vector vEndPoint = GetEndPoint();

	Vector vecForward;
	Vector vecRight;
	Vector vecUp;
	AngleVectors( GetNetworkAngles(), &vecForward, &vecRight, &vecUp );

	float fMaximumTime = fmaxf( prediction->InPrediction() ? prediction->GetSavedTime() : gpGlobals->curtime, engine->GetLastTimeStamp() );

	CPhysConvex *pTempConvex;

	if (sv_thinnerprojectedwalls.GetInt())
	{
		// Generates an infinitely thin light bridge out of 4 vertices

		Vector vBackRight = vStartPoint + (vecRight * (PROJECTED_WALL_WIDTH / 2));
		Vector vBackLeft = vStartPoint - (vecRight * (PROJECTED_WALL_WIDTH / 2));
		Vector vFrontRight = vEndPoint + (vecRight * (PROJECTED_WALL_WIDTH / 2));
		Vector vFrontLeft = vEndPoint - (vecRight * (PROJECTED_WALL_WIDTH / 2));

		Vector *vVerts[4];

		vVerts[0] = &vBackRight;
		vVerts[1] = &vBackLeft;
		vVerts[2] = &vFrontRight;
		vVerts[3] = &vFrontLeft;
		
		pTempConvex = physcollision->ConvexFromVerts( vVerts, 4 );
	}
	else
	{
		// Generates a 1/64 unit thick light bridge out of 6 planes
		// Based on how Valve uses GeneratePolyhedronFromPlanes elsewhere they probably just did everything in-line like this

		float fPlanes[6 * 4];

		// Forward plane
		fPlanes[(0 * 4) + 0] = vecForward.x;
		fPlanes[(0 * 4) + 1] = vecForward.y;
		fPlanes[(0 * 4) + 2] = vecForward.z;
		fPlanes[(0 * 4) + 3] = vecForward.Dot(vEndPoint);

		// Back plane
		fPlanes[(1 * 4) + 0] = -vecForward.x;
		fPlanes[(1 * 4) + 1] = -vecForward.y;
		fPlanes[(1 * 4) + 2] = -vecForward.z;
		fPlanes[(1 * 4) + 3] = -vecForward.Dot(vStartPoint);

		// Right plane
		fPlanes[(4 * 4) + 0] = vecRight.x;
		fPlanes[(4 * 4) + 1] = vecRight.y;
		fPlanes[(4 * 4) + 2] = vecRight.z;
		fPlanes[(4 * 4) + 3] = vecRight.Dot( vStartPoint + (vecRight * PROJECTED_WALL_WIDTH / 2) );

		// Left plane
		fPlanes[(5 * 4) + 0] = -vecRight.x;
		fPlanes[(5 * 4) + 1] = -vecRight.y;
		fPlanes[(5 * 4) + 2] = -vecRight.z;
		fPlanes[(5 * 4) + 3] = -vecRight.Dot( vStartPoint - (vecRight * PROJECTED_WALL_WIDTH / 2) );

		// Up plane
		fPlanes[(2 * 4) + 0] = vecUp.x;
		fPlanes[(2 * 4) + 1] = vecUp.y;
		fPlanes[(2 * 4) + 2] = vecUp.z;
		fPlanes[(2 * 4) + 3] = vecUp.Dot( vStartPoint + (vecUp * PROJECTED_WALL_HEIGHT / 2) );

		// Down plane
		fPlanes[(3 * 4) + 0] = -vecUp.x;
		fPlanes[(3 * 4) + 1] = -vecUp.y;
		fPlanes[(3 * 4) + 2] = -vecUp.z;
		fPlanes[(3 * 4) + 3] = -vecUp.Dot( vStartPoint - (vecUp * PROJECTED_WALL_HEIGHT / 2) );

		CPolyhedron *pPolyhedron = GeneratePolyhedronFromPlanes( fPlanes, 6, 0.0 );
		if (!pPolyhedron)
		{
			Warning( "CProjectedWallEntity: GeneratePolyhedronFromPlanes failed! Get a save game for me!.\n" );
			return;
		}
		pTempConvex = physcollision->ConvexFromConvexPolyhedron( *pPolyhedron );
		pPolyhedron->Release();
	}

	if (!pTempConvex)
		return;

	bool bSetLength = true;

	CPhysCollide *pWallCollideable = physcollision->ConvertConvexToCollide( &pTempConvex, 1 );
	if ( pWallCollideable )
	{
		Vector vMaxs = vec3_origin;
		Vector vMins = vec3_origin;
		physcollision->CollideGetAABB(&vMins, &vMaxs, pWallCollideable, vec3_origin, vec3_angle);

		WallCollideableAtTime_t collideable;
		collideable.vStart = GetStartPoint();
		collideable.vEnd = GetEndPoint();
		collideable.vWorldMins = vMins;
		collideable.vWorldMaxs = vMaxs;
		collideable.qAngles = GetNetworkAngles();
		collideable.pCollideable = pWallCollideable;
		collideable.flTime = fMaximumTime;
		m_WallCollideables.AddToTail( collideable );

		// FIXME:
		//m_vWorldSpace_WallMins = vMins;
		//m_vWorldSpace_WallMaxs = vMaxs;

		// set entity size
		Vector vLocalMins = vMins - vStartPoint;
		Vector vLocalMaxs = vMaxs - vStartPoint;
		SetSize( vLocalMins, vLocalMaxs );

		// Unsure if they actually used this function or not...original decompiled code below
		m_flLength = vStartPoint.DistTo(vEndPoint);
		//m_flLength = sqrt(
		//	(((vStartPoint.x - vEndPoint.x) * (vStartPoint.x - vEndPoint.x))
		//	+ ((vStartPoint.y - vEndPoint.y) * (vStartPoint.y - vEndPoint.y)))
		//	+ ((vStartPoint.z - vEndPoint.z) * (vStartPoint.z - vEndPoint.z)));
				
		if ( bSetLength )
		{
			m_flCurDisplayLength = 0.0;
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}

		CollisionProp()->MarkSurroundingBoundsDirty();
		CollisionProp()->MarkPartitionHandleDirty();
				
		if ( prediction->InPrediction() )
		{
			CheckForPlayersOnBridge();
			DisplaceObstructingEntities();
		}

		m_nNumSegments = ceil( ( m_flLength / m_flSegmentLength ) );
		m_PaintPowers.SetCount( m_nNumSegments );
		CleansePaint();
	}
}

void C_ProjectedWallEntity::RestoreToToolRecordedState( KeyValues *pKV )
{
	/*matrix3x4_t* pmVar2;
	Vector local_28;
	float local_1c;
	float local_18;
	float local_14;
	QAngle local_10;

	//field_0xadc = pKV->GetFloat("sp_x");
	//field_0xae0 = pKV->GetFloat("sp_y");
	//field_0xae4 = pKV->GetFloat("sp_z");
	m_flWidth = pKV->GetFloat("width");
	m_flHeight = pKV->GetFloat("height");
	m_nNumSegments = pKV->GetInt("numSegments", 0);
	m_flSegmentLength = pKV->GetFloat("segLength", 0.0);
	m_flCurDisplayLength = pKV->GetFloat("dispLength", 0.0);
	m_vWorldSpace_WallMins.x = -16384.0;
	m_vWorldSpace_WallMins.y = -16384.0;
	m_vWorldSpace_WallMins.z = -16384.0;
	m_vWorldSpace_WallMaxs.x = 16384.0;
	m_vWorldSpace_WallMaxs.y = 16384.0;
	m_vWorldSpace_WallMaxs.z = 16384.0;
	SetSize(m_vWorldSpace_WallMins, m_vWorldSpace_WallMaxs);
	local_18 = pKV->GetFloat("na_y", 0.0);
	local_1c = pKV->GetFloat("na_p", 0.0);
	local_14 = pKV->GetFloat("na_r", 0.0);
	pmVar2 = pKV->GetPtr("worldTransform");
	if (pmVar2) 
	{
		MatrixAngles(pmVar2, local_10);
		local_28.x = pmVar2->m_flMatVal[0][3];
		local_28.y = pmVar2->m_flMatVal[1][3];
		local_28.z = pmVar2->m_flMatVal[2][3];
		local_10.x = local_1c;
		local_10.y = local_18;
		local_10.z = local_14;
		SetNetworkOrigin(local_28);
		SetAbsOrigin(local_28);
		SetLocalOrigin(local_28);
		SetNetworkAngles(local_10);
		SetAbsAngles(local_10);
		SetLocalAngles(local_10);
	}
	if (&this->field_0x36c != (undefined1*)0x0) 
	{
		*(undefined4*)&this->field_0x36c = 0xffffffff;
	}*/
}

void C_ProjectedWallEntity::SetPaintPower( int nSegment, PaintPowerType power )
{

}

void C_ProjectedWallEntity::UpdateOnRemove( void )
{
	if ( prediction->InPrediction() )
		CheckForPlayersOnBridge();

	StopParticleEffects( this );
	for ( int i = 0; i < m_WallCollideables.Count(); ++i )
	{
		CPhysCollide *pCollideable = m_WallCollideables[i].pCollideable;
		physcollision->DestroyCollide( pCollideable );
	}

	m_WallCollideables.RemoveAll();
	BaseClass::UpdateOnRemove();
}

bool C_ProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace )
{
	if ( !m_pActiveCollideable )
		return false;

	physcollision->TraceBox( ray, mask, NULL, m_pActiveCollideable, vec3_origin, vec3_angle, &trace );
	return trace.fraction < 1.0 || trace.allsolid || trace.startsolid;
}

bool C_ProjectedWallEntity::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	return TestCollision( ray, fContentsMask, tr );
}

const QAngle &C_ProjectedWallEntity::GetRenderAngles( void )
{
	return vec3_angle;
}

void C_ProjectedWallEntity::ClientThink( void )
{
	BaseClass::ClientThink();

	float speed = cl_projected_wall_projection_speed.GetFloat();
	float v1 = 0;
	float v2 = 0;

	if ( speed >= 0.0 )
		v2 = fminf( m_flLength, speed );

	float v4 = v2 + m_flCurDisplayLength;
	if ( v4 >= 0 )
		v1 = fminf( m_flLength, v4 );

	m_flCurDisplayLength = v1;
	if ( m_flParticleUpdateTime != m_flPrevParticleUpdateTime )
	{
		m_flPrevParticleUpdateTime = m_flParticleUpdateTime;
		SetupWallParticles();
	}
	
	SetNextClientThink( gpGlobals->curtime + 0.016 );
}

bool C_ProjectedWallEntity::InitMaterials( void )
{
	if ( !m_pBodyMaterial )
	{
		m_pBodyMaterial = materials->FindMaterial( "effects/projected_wall", NULL, false );
		if ( !m_pBodyMaterial )
			return false;
	}
	
	if ( !m_pSideRailMaterial )
	{
		m_pSideRailMaterial = materials->FindMaterial( "effects/projected_wall_rail", NULL, false );
		if ( !m_pSideRailMaterial )
			return false;
	}

	// UNDONE FUNCTION

	return true;
}

bool C_ProjectedWallEntity::ShouldSpawnParticles( C_Portal_Base2D *pPortal )
{
	if ( !pPortal->IsActivedAndLinked() )
		return true;

	if ( !dynamic_cast<C_Prop_Portal*>( pPortal ) )
		return false;

	Vector vPortalUp;
	AngleVectors( pPortal->GetAbsAngles(), NULL, NULL, &vPortalUp );
	
	Vector vWallUp;
	AngleVectors( GetAbsAngles(), NULL, NULL, &vWallUp );

	float dot = DotProduct( vPortalUp, vWallUp );
	if ( dot < -1.0 )
	{
		dot = -1.0;
	}
	else if ( dot > 1.0 )
	{
		dot = 1.0;
	}
	
	return fabs( dot ) > STEEP_SLOPE;
}

void C_ProjectedWallEntity::SetupWallParticles()
{
	StopParticleEffects( this );

	C_Portal_Base2D *pSourcePortal = m_hSourcePortal;
	C_Portal_Base2D *pHitPortal = m_hHitPortal;

	Vector vWallEndPoint = GetEndPoint();
	Vector vWallStartPoint = GetStartPoint();

	Vector vecForward, vecRight, vecUp;
	QAngle qAngles = GetNetworkAngles();
	AngleVectors( qAngles, &vecForward, &vecRight, &vecUp );

	if ( pSourcePortal && ShouldSpawnParticles( pSourcePortal ) )
	{
		Vector vecPortalPos;
		C_Prop_Portal *pPropPortal = dynamic_cast<C_Prop_Portal*>( pSourcePortal );
		if ( pPropPortal )
		{
			vecPortalPos = pPropPortal->GetAbsOrigin();
		}
		else
		{
			vecPortalPos = vWallStartPoint;
			vecPortalPos.z += 512;
		}

		Vector particleOrg = (vecForward * 3.125) + vWallStartPoint;
		DispatchParticleEffect( "projected_wall_impact", particleOrg, vecPortalPos, qAngles, this );
	}
	
	if ( !pHitPortal || ShouldSpawnParticles( pHitPortal ) )
	{
		Vector vecPortalPos;
		C_Prop_Portal *pPropPortal = dynamic_cast<C_Prop_Portal*>( pHitPortal );
		if ( pPropPortal )
		{
			vecPortalPos = pPropPortal->GetAbsOrigin();
		}
		else
		{
			vecPortalPos = vWallEndPoint;
			vecPortalPos.z += 512;
		}

		Vector particleOrg = vWallEndPoint - (vecForward * 3.125);
		DispatchParticleEffect( "projected_wall_impact", particleOrg, vecPortalPos, qAngles, this );
	}
}

void C_ProjectedWallEntity::CheckForPlayersOnBridge()
{
	for (int iVar1 = 0; iVar1 != -1; iVar1 = NextValidSplitScreenSlot(iVar1)) 
	{
		C_Portal_Player * this_00 = (C_Portal_Player*)C_BasePlayer::GetLocalPlayer(iVar1);
		if (this_00)
		{
			C_ProjectedWallEntity* pCVar2 = (C_ProjectedWallEntity *)this_00->GetGroundEntity();
			if(pCVar2 == this) 
			{
					this_00->SetGroundEntity(NULL);
					this_00->BridgeRemovedFromUnder();
			}
		}
	}
}

RenderableTranslucencyType_t C_ProjectedWallEntity::ComputeTranslucencyType( void )
{
	return RENDERABLE_IS_TRANSLUCENT;
}


bool C_ProjectedWallEntity::SetPaintSurfaceColor(PaintPowerType& param_1)

{
	PaintPowerType PVar1;
	float local_10;
	float local_c;
	float local_8;

	PVar1 = param_1;
	local_10 = 1.0;
	local_c = 1.0;
	local_8 = 1.0;
	if (PVar1 != NO_POWER) 
	{
		MapPowerToVisualColor((int)&param_1);
		local_10 = (float)((uint)param_1 & 0xff) * 0.003921569;
		local_c = (float)((uint)param_1 >> 8 & 0xff) * 0.003921569;
		local_8 = (float)((uint)param_1 >> 0x10 & 0xff) * 0.003921569;
	}
	m_pPaintColorMid->SetVecValue(&local_10, 3);
	m_pPaintColorEnd1->SetVecValue(&local_10, 3);
	m_pPaintColorEnd2->SetVecValue(&local_10, 3);
	m_pPaintColorSing->SetVecValue(&local_10, 3);
	return PVar1 != NO_POWER;
}


int C_ProjectedWallEntity::DrawModel( int flags, const RenderableInstance_t &instance )
{
	if ( false ) // Unknown condition
		return 0;

	if ( engine->HasPaintmap() &&
		cl_draw_projected_wall_with_paint.GetBool() &&
		m_PaintPowers.NumAllocated() == m_nNumSegments ) // FIXME: Compare m_nNumSegments to m_PaintPowers.m_Memory.m_nGrowSize instead
	{/*
		pIVar5 = (IMatRenderContext*)(**(code**)((int)*materials + 0x1c4))();
		if (pIVar5 != (IMatRenderContext*)0x0) {
			(**(code**)((int)*pIVar5 + 8))();
		}
		PaintWallWithPaint<0>
			((C_ProjectedWallEntity*)this_00, *(IMaterial**)&this->field_0xb10, pIVar5, NO_POWER);
		PaintWallWithPaint<1>
			((C_ProjectedWallEntity*)this_00, (IMaterial*)this->m_pPaintColorSing, pIVar5, NO_POWER
			);
		fVar11 = (this->m_vLastStartpos).z;
		local_20[0] = '\0';
		local_20[1] = '\0';
		local_20[2] = '\0';
		local_20[3] = '\0';
		local_20[4] = '\0';
		if (0 < (int)fVar11) {
			piVar8 = (int*)this->m_flSegmentLength;
			do {
				iVar10 = *piVar8;
				piVar8 = piVar8 + 1;
				fVar11 = (float)((int)fVar11 + -1);
				local_20[iVar10] = '\x01';
			} while (fVar11 != 0.0);
		}
		local_8 = 0.0;
		local_c = this->m_pPaintMaterialsMid + 3;
		do {
			if (local_20[(int)local_8] != '\0') {
				PaintWallWithPaint<4>
					((C_ProjectedWallEntity*)this_00, local_c[-4], pIVar5, (PaintPowerType)local_8);
				PaintWallWithPaint<2>
					((C_ProjectedWallEntity*)this_00, *local_c, pIVar5, (PaintPowerType)local_8);
				PaintWallWithPaint<3>
					((C_ProjectedWallEntity*)this_00, local_c[4], pIVar5, (PaintPowerType)local_8);
				PaintWallWithPaint<5>
					((C_ProjectedWallEntity*)this_00, local_c[8], pIVar5, (PaintPowerType)local_8);
			}
			local_c = local_c + 1;
			local_8 = (float)((int)local_8 + REFLECT_POWER);
		} while ((int)local_8 < 4);
		if ((local_20[0] != '\0') && (local_20[2] != '\0')) {
			PaintWallWithPaint<6>
				((C_ProjectedWallEntity*)this_00, this->m_pPaintMaterialsSing[3], pIVar5, SPEED_POWER)
				;
			PaintWallWithPaint<7>
				((C_ProjectedWallEntity*)this_00, this->m_pPaintMaterialBounceLSpeed, pIVar5,
					SPEED_POWER);
			PaintWallWithPaint<8>
				((C_ProjectedWallEntity*)this_00, this->m_pPaintMaterialBounceRSpeed, pIVar5,
					SPEED_POWER);
			PaintWallWithPaint<9>
				((C_ProjectedWallEntity*)this_00, this->m_pPaintMaterialBounceLRSpeed, pIVar5,
					SPEED_POWER);
			PaintWallWithPaint<10>
				((C_ProjectedWallEntity*)this_00, this->m_pPaintMaterialRBounceLSpeed, pIVar5,
					SPEED_POWER);
		}
		if (pIVar5 != (IMatRenderContext*)0x0) 
		{
			(**(code**)((int)*pIVar5 + 0xc))();
			(**(code**)((int)*pIVar5 + 4))();
			return 1;
		}*/
	}
	else
	{
		Vector vForward, vRight, vUp;
		AngleVectors( GetNetworkAngles(), &vForward, &vRight, &vUp );

		Vector vOrigin = GetStartPoint() + ( ( vForward * m_flCurDisplayLength ) * 0.5 );

		CMatRenderContextPtr pRenderContext( materials );
		IMesh *pBodyMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pBodyMaterial );

		CMeshBuilder meshBuilder;
		if ( g_pGameRules->IsMultiplayer() )
		{
			// Nothing, probably leftover code
		}

		float flLengthTexRate = m_flCurDisplayLength * 0.0078125;

		Vector vHalfLength = vForward * ( ( m_flCurDisplayLength * 0.5 ) + 0.1 );
		Vector vHalfWidth = ( vRight * m_flWidth ) * 0.5;
		Vector vHalfHeight = ( vUp * m_flHeight ) * 0.5;

		meshBuilder.Begin( pBodyMesh, MATERIAL_QUADS, 50 );
		Vector vStart = vHalfHeight + vOrigin;
		DrawQuadHelper<false>( &meshBuilder, vStart, vHalfLength, vHalfWidth, 1.0, flLengthTexRate );
		meshBuilder.End();
		pBodyMesh->Draw();

		IMesh *pRailMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pSideRailMaterial );
		meshBuilder.Begin( pRailMesh, MATERIAL_QUADS, 20 );

		vStart = vHalfWidth + vOrigin;
		vHalfHeight.z = vUp.z * 6.0;
		DrawQuadHelper<false>( &meshBuilder, vStart, vHalfLength, vHalfHeight, 1.0, flLengthTexRate );
		vStart = vOrigin - vHalfWidth;
		DrawQuadHelper<false>( &meshBuilder, vStart, vHalfLength, vHalfHeight, 1.0, flLengthTexRate );		

		meshBuilder.End();
		pRailMesh->Draw();
	}

	return 1;
}

void C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	*pWorldMins = m_vWorldSpace_WallMins;
	*pWorldMaxs = m_vWorldSpace_WallMaxs;
}

void C_ProjectedWallEntity::GetRenderBounds( Vector& vecMins, Vector& vecMaxs )
{
	vecMins = m_vWorldSpace_WallMins - GetRenderOrigin();
	vecMaxs = m_vWorldSpace_WallMaxs - GetRenderOrigin();
}

void C_ProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector& outMaxs )
{
	GetExtents( outMins, outMaxs, 0.5 );
}

bool C_ProjectedWallEntity::ShouldDraw( void )
{
	return true;
}

CollideType_t C_ProjectedWallEntity::GetCollideType( void )
{
	return ENTITY_SHOULD_COLLIDE;
}

void C_ProjectedWallEntity::GetToolRecordingState( KeyValues *msg )
{
	BaseClass::GetToolRecordingState( msg );
}