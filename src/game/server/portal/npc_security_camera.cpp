//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_senses.h"
#include "ai_memory.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "Sprite.h"
#include "portal_player.h"
#include "soundenvelope.h"
#include "explode.h"
#include "IEffects.h"
#include "animation.h"
#include "props.h"
#include "rope.h"
#include "rope_shared.h"
#include "basehlcombatweapon_shared.h"
#include "iservervehicle.h"
#include "physics_prop_ragdoll.h"
#include "portal_util_shared.h"
#include "prop_portal.h"
#include "portal_player.h"
#include "world.h"
#include "GameEventListener.h"
#include "portal_mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



//Datatable
BEGIN_DATADESC( CNPC_SecurityCamera )

	DEFINE_FIELD( m_hEyeGlow,		FIELD_EHANDLE ),

	DEFINE_FIELD( m_bAutoStart,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bActive,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBlinkState,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEnabled,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastSight,		FIELD_TIME ),
	DEFINE_FIELD( m_flPingTime,			FIELD_TIME ),
	DEFINE_FIELD( m_vecGoalAngles,		FIELD_VECTOR ),
	DEFINE_FIELD( m_vecCurrentAngles,	FIELD_VECTOR ),
	DEFINE_FIELD( m_vNoisePos,			FIELD_VECTOR ),
	DEFINE_FIELD( m_iTicksTillNextNoise, FIELD_INTEGER ),

	// Portal 2 Stuff
	DEFINE_KEYFIELD(m_bLookAtPlayerPings, FIELD_BOOLEAN, "LookAtPlayerPings"),
	DEFINE_KEYFIELD(m_nTeamToLookAt, FIELD_INTEGER, "TeamToLookAt"),
	DEFINE_KEYFIELD(m_nTeamPlayerToLookAt, FIELD_INTEGER, "TeamPlayerToLookAt"),

	DEFINE_SOUNDPATCH( m_pMovementSound ),

	DEFINE_THINKFUNC( Retire ),
	DEFINE_THINKFUNC( Deploy ),
	DEFINE_THINKFUNC( ActiveThink ),
	DEFINE_THINKFUNC( DormantThink ),
	DEFINE_THINKFUNC( SearchThink ),
	DEFINE_THINKFUNC( DeathThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Ragdoll", InputRagdoll ),

	// Coop stuff
	DEFINE_INPUTFUNC( FIELD_VOID, "LookAtBlue", InputLookAtBlue ),
	DEFINE_INPUTFUNC( FIELD_VOID, "LookAtOrange", InputLookAtOrange ),
	DEFINE_INPUTFUNC( FIELD_VOID, "LookAllTeams", InputLookAllTeams ),


	DEFINE_OUTPUT( m_OnDeploy, "OnDeploy" ),
	DEFINE_OUTPUT( m_OnRetire, "OnRetire" ),

	DEFINE_OUTPUT( m_OnTaunted, "OnTaunted"),
	DEFINE_OUTPUT( m_OnTauntedBlue, "OnTauntedBlue"),
	DEFINE_OUTPUT( m_OnTauntedOrange, "OnTauntedOrange"),
	DEFINE_OUTPUT( m_OnTauntedFinished, "OnTauntedFinished"),
	DEFINE_OUTPUT( m_OnTauntedBlueFinished, "OnTauntedBlueFinished"),
	DEFINE_OUTPUT( m_OnTauntedOrangeFinished, "OnTauntedOrangeFinished"),

END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_security_camera, CNPC_SecurityCamera );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CNPC_SecurityCamera::CNPC_SecurityCamera( void )
{
	m_bActive			= false;
	m_bAutoStart		= false;
	m_flPingTime		= 0;
	m_flLastSight		= 0;
	m_bBlinkState		= false;
	m_bEnabled			= false;
	m_vecCurrentAngles	= QAngle( 0.0f, 0.0f, 0.0f );

	m_vecPingLocation   = Vector( 0.0f, 0.0f, 0.0f );

	m_vecGoalAngles.Init();
	m_vNoisePos = Vector( 0.0f, 0.0f, 0.0f );
	m_iTicksTillNextNoise = 5;

	m_pMovementSound = NULL;
	m_hEyeGlow = NULL;

	m_bDetectedNewPing = false;
}

CNPC_SecurityCamera::~CNPC_SecurityCamera( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Precache( void )
{
	PrecacheModel( SECURITY_CAMERA_MODEL );	

	PrecacheScriptSound( "Portalgun.pedestal_rotate_loop" );
	
	BaseClass::Precache();
}

void CNPC_SecurityCamera::CreateSounds()
{
	if (!m_pMovementSound)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		CPASAttenuationFilter filter( this );

		m_pMovementSound = controller.SoundCreate( filter, entindex(), "Portalgun.pedestal_rotate_loop" );
		controller.Play( m_pMovementSound, 0, 100 );
	}
}

void CNPC_SecurityCamera::StopLoopingSounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy( m_pMovementSound );
	m_pMovementSound = NULL;

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Spawn( void )
{ 
	Precache();

	SetModel( SECURITY_CAMERA_MODEL );
	
	BaseClass::Spawn();

	m_HackedGunPos	= Vector( 0, 0, 12.75 );
	SetViewOffset( EyeOffset( ACT_IDLE ) );
	m_flFieldOfView	= VIEW_FIELD_FULL;
	m_takedamage	= DAMAGE_NO;
	m_iHealth		= 1000;
	m_bloodColor	= BLOOD_COLOR_MECH;
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	SetCollisionBounds( Vector( -16.0f, -16.0f, -16.0f ), Vector( 16.0f, 16.0f, 16.0f ) );

	RemoveFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( SECURITY_CAMERA_BC_YAW, 0 );
	SetPoseParameter( SECURITY_CAMERA_BC_PITCH, 0 );

	//Set our autostart state
	m_bAutoStart = !!( m_spawnflags & SF_SECURITY_CAMERA_AUTOACTIVATE );
	m_bEnabled	 = ( ( m_spawnflags & SF_SECURITY_CAMERA_STARTINACTIVE ) == false );

	//Do we start active?
	if ( m_bAutoStart && m_bEnabled )
	{
		SetThink( &CNPC_SecurityCamera::SearchThink );
	}
	else
	{
		SetThink( &CNPC_SecurityCamera::DormantThink );
	}

	//Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	if (m_bLookAtPlayerPings)
	{
		ListenForGameEvent("portal_player_ping");
	}

	CreateVPhysics();

	SetFadeDistance(-1.0f, 0.0f);
	SetGlobalFadeScale(0.0f);
}

void CNPC_SecurityCamera::Activate( void )
{
	BaseClass::Activate();

	CreateSounds();

	EyeOn();
}

bool CNPC_SecurityCamera::CreateVPhysics( void )
{
	IPhysicsObject *pPhysics = VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_NOT_STANDABLE, false );
	if ( !pPhysics )
		// Technically inaccurate - this still refers to npc_floor_turret in portal 2, changed it for clarity if this ever comes up
		DevMsg( "npc_security_camera unable to spawn physics object!\n" );
	else
		pPhysics->EnableMotion( false );

	return true;
}

void CNPC_SecurityCamera::UpdateOnRemove( void )
{
	EyeOff();

	BaseClass::UpdateOnRemove();
}

int CNPC_SecurityCamera::ObjectCaps( void )
{
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( !pPhysics || !pPhysics->IsMotionEnabled() )
		return BaseClass::ObjectCaps();

	return ( BaseClass::ObjectCaps() | FCAP_USE_IN_RADIUS | FCAP_USE_ONGROUND | FCAP_IMPULSE_USE );
}

void CNPC_SecurityCamera::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if ( pPlayer )
		pPlayer->PickupObject( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_SecurityCamera::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	if ( !m_takedamage )
		return 0;

	CTakeDamageInfo info = inputInfo;

	if ( m_bActive == false )
		info.ScaleDamage( 0.1f );

	m_iHealth -= info.GetDamage();

	if ( m_iHealth <= 0 )
	{
		m_iHealth = 0;
		m_takedamage = DAMAGE_NO;

		RemoveFlag( FL_NPC ); // why are they set in the first place???

		ExplosionCreate( GetAbsOrigin(), GetLocalAngles(), this, 100, 100, false );
		SetThink( &CNPC_SecurityCamera::DeathThink );

		StopSound( "NPC_SecurityCamera.Alert" );

		m_OnDamaged.FireOutput( info.GetInflictor(), this );

		SetNextThink( gpGlobals->curtime + 0.1f );

		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: We override this code because otherwise we start to move into the
//			tricky realm of player avoidance.  Since we don't go through the
//			normal NPC thinking but we ARE an NPC (...) we miss a bunch of 
//			book keeping.  This means we can become invisible and then never
//			reappear.
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::PlayerPenetratingVPhysics( void )
{
	// We don't care!
}

bool CNPC_SecurityCamera::OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	return !m_bActive;
}

//-----------------------------------------------------------------------------
// Purpose: Shut down
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Retire( void )
{
	if ( PreThink( CAMERA_RETIRING ) )
		return;

	//Level out the turret
	m_vecGoalAngles = GetAbsAngles();
	SetNextThink( gpGlobals->curtime );

	//Set ourselves to close
	if ( m_bActive )
	{
		//Notify of the retraction
		m_OnRetire.FireOutput( NULL, this );
	}

	m_bActive		= false;
	m_flLastSight	= 0;

	SetThink( &CNPC_SecurityCamera::SUB_DoNothing );
}

//-----------------------------------------------------------------------------
// Purpose: Start up
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Deploy( void )
{
	if ( PreThink( CAMERA_DEPLOYING ) )
		return;

	m_vecGoalAngles = GetAbsAngles();

	SetNextThink( gpGlobals->curtime );

	if ( !m_bActive )
	{
		m_bActive = true;

		//Notify we're deploying
		m_OnDeploy.FireOutput( NULL, this );
	}

	m_flPlaybackRate = 0;
	SetThink( &CNPC_SecurityCamera::SearchThink );

	//EmitSound( "NPC_SecurityCamera.Move" );

	SetLastSightTime();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::SetLastSightTime()
{
	if( HasSpawnFlags( SF_SECURITY_CAMERA_NEVERRETIRE ) )
	{
		m_flLastSight = FLT_MAX;
	}
	else
	{
		m_flLastSight = gpGlobals->curtime + SECURITY_CAMERA_MAX_WAIT;	
	}
}

//-----------------------------------------------------------------------------
// Purpose: Causes the turret to face its desired angles
//-----------------------------------------------------------------------------
bool CNPC_SecurityCamera::UpdateFacing( void )
{
	bool  bMoved = false;

	float flTurnSpeed = m_bLookAtPlayerPings ? SECURITY_CAMERA_YAW_SPEED_PING : SECURITY_CAMERA_YAW_SPEED;

	if ( m_vecCurrentAngles.x < m_vecGoalAngles.x )
	{
		m_vecCurrentAngles.x += flTurnSpeed;

		if ( m_vecCurrentAngles.x > m_vecGoalAngles.x )
			m_vecCurrentAngles.x = m_vecGoalAngles.x;

		bMoved = true;
	}

	if ( m_vecCurrentAngles.y < m_vecGoalAngles.y )
	{
		m_vecCurrentAngles.y += flTurnSpeed;

		if ( m_vecCurrentAngles.y > m_vecGoalAngles.y )
			m_vecCurrentAngles.y = m_vecGoalAngles.y;

		bMoved = true;
	}

	if ( m_vecCurrentAngles.x > m_vecGoalAngles.x )
	{
		m_vecCurrentAngles.x -= flTurnSpeed;

		if ( m_vecCurrentAngles.x < m_vecGoalAngles.x )
			m_vecCurrentAngles.x = m_vecGoalAngles.x;

		bMoved = true;
	}

	if ( m_vecCurrentAngles.y > m_vecGoalAngles.y )
	{
		m_vecCurrentAngles.y -= flTurnSpeed;

		if ( m_vecCurrentAngles.y < m_vecGoalAngles.y )
			m_vecCurrentAngles.y = m_vecGoalAngles.y;

		bMoved = true;
	}

	if ( bMoved )
	{
		if ( m_pMovementSound )
		{
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

			controller.SoundChangeVolume( m_pMovementSound, RandomFloat( 0.7f, 0.9f ), 0.05f );
		}

		// Update pitch
		int iPose = LookupPoseParameter( SECURITY_CAMERA_BC_PITCH );
		SetPoseParameter( iPose, m_vecCurrentAngles.x );

		// Update yaw
		iPose = LookupPoseParameter( SECURITY_CAMERA_BC_YAW );
		SetPoseParameter( iPose, m_vecCurrentAngles.y );

		InvalidateBoneCache();
	}
	else
	{
		if ( m_pMovementSound )
		{
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

			controller.SoundChangeVolume( m_pMovementSound, 0.0f, 0.05f );
		}
	}

	return bMoved;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_SecurityCamera::FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
	CBaseEntity	*pHitEntity = NULL;
	if ( BaseClass::FVisible( pEntity, traceMask, &pHitEntity ) )
		return true;

	// If we hit something that's okay to hit anyway, still fire
	if ( pHitEntity && pHitEntity->MyCombatCharacterPointer() )
	{
		if (IRelationType(pHitEntity) == D_HT)
			return true;
	}

	if (ppBlocker)
	{
		*ppBlocker = pHitEntity;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Allows the turret to fire on targets if they're visible
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::ActiveThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( CAMERA_ACTIVE ) )
		return;

	//Update our think time
	SetNextThink( gpGlobals->curtime + 0.1f );

	CBaseEntity *pEnemy = GetEnemy();

	//If we've become inactive, go back to searching
	if ( m_bActive == false || !pEnemy )
	{
		SetEnemy( NULL );
		SetLastSightTime();
		SetThink( &CNPC_SecurityCamera::SearchThink );
		m_vecGoalAngles = GetAbsAngles();
		return;
	}
	
	//Get our shot positions
	Vector vecMid = EyePosition();
	Vector vecMidEnemy = pEnemy->GetAbsOrigin();

	if (m_bLookAtPlayerPings)
		vecMidEnemy = m_vecPingLocation;

	//Store off our last seen location
	UpdateEnemyMemory( pEnemy, vecMidEnemy );

	//Look for our current enemy
	bool bEnemyVisible = m_bLookAtPlayerPings || (pEnemy->IsAlive() && FInViewCone( pEnemy ) && FVisible( pEnemy ));

	//Calculate dir and dist to enemy
	Vector	vecDirToEnemy = vecMidEnemy - vecMid;	
	float	flDistToEnemy = VectorNormalize( vecDirToEnemy );

	CPortal_Base2D *pPortal = NULL;

	if ( pEnemy->IsAlive() && !m_bLookAtPlayerPings )
	{
		pPortal = FInViewConeThroughPortal( pEnemy );

		if ( pPortal && FVisibleThroughPortal( pPortal, pEnemy ) )
		{
			// Translate our target across the portal
			Vector vecMidEnemyTransformed;
			UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vecMidEnemy, vecMidEnemyTransformed );

			//Calculate dir and dist to enemy
			Vector	vecDirToEnemyTransformed = vecMidEnemyTransformed - vecMid;	
			float	flDistToEnemyTransformed = VectorNormalize( vecDirToEnemyTransformed );

			// If it's not visible through normal means or the enemy is closer through the portal, use the translated info
			if ( !bEnemyVisible || flDistToEnemyTransformed < flDistToEnemy )
			{
				bEnemyVisible = true;
				vecMidEnemy = vecMidEnemyTransformed;
				vecDirToEnemy = vecDirToEnemyTransformed;
				flDistToEnemy = flDistToEnemyTransformed;
			}
			else
			{
				pPortal = NULL;
			}
		}
		else
		{
			pPortal = NULL;
		}
	}

	// Add noise to the look position
	--m_iTicksTillNextNoise;

	if ( m_iTicksTillNextNoise <= 0 && flDistToEnemy < 256.0f )
	{
		m_vNoisePos.x = RandomFloat( -8.0f, 8.0f );
		m_vNoisePos.y = RandomFloat( -8.0f, 8.0f );
		m_vNoisePos.z = RandomFloat( 0.0f, 32.0f );

		m_iTicksTillNextNoise = RandomInt( 5, 30 );
	}

	//We want to look at the enemy's eyes so we don't jitter
	Vector vEnemyEyes = pEnemy->EyePosition();

	if ( m_bLookAtPlayerPings )
		vEnemyEyes = m_vecPingLocation;

	if ( pPortal )
	{
		UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vEnemyEyes, vEnemyEyes );
	}

	Vector	vecDirToEnemyEyes = ( vEnemyEyes + m_vNoisePos ) - vecMid;
	VectorNormalize( vecDirToEnemyEyes );

	QAngle vecAnglesToEnemy;
	VectorAngles( vecDirToEnemyEyes, vecAnglesToEnemy );

	Vector vForward, vRight, vUp;
	GetVectors( &vForward, &vRight, &vUp );

	vecAnglesToEnemy.x = acosf( vecDirToEnemyEyes.Dot( -vUp ) ) * ( 180.0f / M_PI );

	Vector vProjectedDirToEnemyEyes = vecDirToEnemyEyes - vecDirToEnemyEyes.Dot( vUp ) * vUp;
	VectorNormalize( vProjectedDirToEnemyEyes );

	if ( vProjectedDirToEnemyEyes.IsZero() )
		vecAnglesToEnemy.y = m_vecGoalAngles.y;
	else
	{
		if ( vProjectedDirToEnemyEyes.Dot( vForward ) > 0.0f )
			vecAnglesToEnemy.y = acosf( vProjectedDirToEnemyEyes.Dot( vRight ) ) * ( 180.0f / M_PI ) - 90.0f;
		else
			vecAnglesToEnemy.y = -acosf( vProjectedDirToEnemyEyes.Dot( vRight ) ) * ( 180.0f / M_PI ) - 90.0f;
	}

	vecAnglesToEnemy.y = AngleNormalize( vecAnglesToEnemy.y );

	//Current enemy is not visible
	if ( ( bEnemyVisible == false ) || ( flDistToEnemy > SECURITY_CAMERA_RANGE ) )
	{
		if ( gpGlobals->curtime > m_flLastSight )
		{
			// Should we look for a new target?
			ClearEnemyMemory();
			SetEnemy( NULL );
			SetLastSightTime();
			SetThink( &CNPC_SecurityCamera::SearchThink );
			m_vecGoalAngles = GetAbsAngles();

			return;
		}

		bEnemyVisible = false;
	}

	//If we can see our enemy, face it
	if ( bEnemyVisible )
	{
		m_vecGoalAngles.y = vecAnglesToEnemy.y;
		m_vecGoalAngles.x = vecAnglesToEnemy.x;

		m_flLastSight = gpGlobals->curtime + 0.5f;
	}

	//Turn to face
	UpdateFacing();

	if (m_bDetectedNewPing
		 && fabs( AngleDiff( m_vecGoalAngles.x, m_vecCurrentAngles.x ) ) <= 1.0f
		 && fabs( AngleDiff( m_vecGoalAngles.y, m_vecCurrentAngles.y ) ) <= 1.0f)
	{
		m_bDetectedNewPing = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Target doesn't exist or has eluded us, so search for one
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::SearchThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( CAMERA_SEARCHING ) )
		return;

	SetNextThink( gpGlobals->curtime + 0.1f );

	//If our enemy has died, pick a new enemy
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->IsAlive() == false ) )
	{
		SetEnemy( NULL );
	}

	//Acquire the target
 	if ( GetEnemy() == NULL )
	{
		CBaseEntity *pEnemy = NULL;
		bool bLookAtTeam = m_nTeamPlayerToLookAt > 0;

		//CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
		for( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( pPlayer && pPlayer->IsAlive() && ( !bLookAtTeam || m_nTeamPlayerToLookAt == pPlayer->GetTeamNumber() ) )
			{
				if ( FInViewCone( pPlayer ) && FVisible( pPlayer ) )
				{
					pEnemy = pPlayer;
					break;
				}
				else
				{
					CPortal_Base2D *pPortal = FInViewConeThroughPortal( pPlayer );
					if ( pPortal && FVisibleThroughPortal( pPortal, pPlayer ) )
					{
						pEnemy = pPlayer;
						break;
					}
				}
			}
		}

		if ( pEnemy )
		{
			SetEnemy( pEnemy );
		}
	}

	//If we've found a target follow it
	if ( GetEnemy() != NULL )
	{
		m_flLastSight = 0;
		m_bActive = true;
		SetThink( &CNPC_SecurityCamera::ActiveThink );

		//EmitSound( "NPC_CeilingTurret.Active" );
		return;
	}

	--m_iTicksTillNextNoise;

	if ( m_iTicksTillNextNoise <= 0 )
	{
		//Display that we're scanning
		m_vecGoalAngles.x = RandomFloat( -10.0f, 30.0f );
		m_vecGoalAngles.y = RandomFloat( -80.0f, 80.0f );

		m_iTicksTillNextNoise = RandomInt( 10, 35 );
	}

	//Turn and ping
	//UpdateFacing();
	Ping();
}

//-----------------------------------------------------------------------------
// Purpose: Allows a generic think function before the others are called
// Input  : state - which state the turret is currently in
//-----------------------------------------------------------------------------
bool CNPC_SecurityCamera::PreThink( cameraState_e state )
{
	CheckPVSCondition();

	//Animate
	StudioFrameAdvance();

	// Hide the sprite if the model alpha is 0.
	// Valve uses invisible cameras in some coop maps to create the viewfinder effect when taunting
	if (m_clrRender.GetA() > 0)
		EyeOn();
	else
		EyeOff();

	if (CheckRestingSurfaceForPortals())
		return true;

	//Do not interrupt current think function
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Make a pinging noise so the player knows where we are
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Ping( void )
{
	//See if it's time to ping again
	if ( m_flPingTime > gpGlobals->curtime )
		return;

	//Ping!
	//EmitSound( "NPC_CeilingTurret.Ping" );

	m_flPingTime = gpGlobals->curtime + SECURITY_CAMERA_PING_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the turret's state
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Toggle( void )
{
	//Toggle the state
	if ( m_bEnabled )
	{
		Disable();
	}
	else 
	{
		Enable();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Enable the turret and deploy
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Enable( void )
{
	m_bEnabled = true;

	// if the turret is flagged as an autoactivate turret, re-enable its ability open self.
	if ( m_spawnflags & SF_SECURITY_CAMERA_AUTOACTIVATE )
	{
		m_bAutoStart = true;
	}

	SetThink( &CNPC_SecurityCamera::Deploy );
	SetNextThink( gpGlobals->curtime + 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: Retire the turret until enabled again
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::Disable( void )
{
	m_bEnabled = false;
	m_bAutoStart = false;

	SetEnemy( NULL );
	SetThink( &CNPC_SecurityCamera::Retire );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CNPC_SecurityCamera::EyeOn( void )
{
	if ( !m_hEyeGlow )
	{
		// Create our eye sprite
		m_hEyeGlow = CSprite::SpriteCreate( SECURITY_CAMERA_GLOW_SPRITE, GetLocalOrigin(), false );
		if ( !m_hEyeGlow )
			return;

		m_hEyeGlow->SetTransparency( kRenderWorldGlow, 255, 0, 0, 128, kRenderFxNoDissipation );
		m_hEyeGlow->SetAttachment( this, LookupAttachment( "light" ) );
		m_hEyeGlow->SetScale( 0.3f, 1.0f );
	}
}

void CNPC_SecurityCamera::EyeOff( void )
{
	if ( m_hEyeGlow != NULL )
	{
		UTIL_Remove( m_hEyeGlow );
		m_hEyeGlow = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::InputToggle( inputdata_t &inputdata )
{
	Toggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::InputEnable( inputdata_t &inputdata )
{
	Enable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

void CNPC_SecurityCamera::Ragdoll()
{
	if ( !m_bEnabled )
		return;

	// Leave decal on wall (may want to disable this once decal for where cam touches wall is made)
	Vector vForward;
	GetVectors( &vForward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine ( GetAbsOrigin() + 10.0f * vForward, GetAbsOrigin() -60.0f * vForward, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.m_pEnt )
		UTIL_DecalTrace( &tr, "SecurityCamera.Detachment" );

	// Disable it's AI
	Disable();
	SetThink( &CNPC_SecurityCamera::DeathThink );
	EyeOff();

	// Make it move
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( !pPhysics || pPhysics->IsMotionEnabled() )
		return;

	pPhysics->EnableMotion( true );
	pPhysics->Wake();
}

void CNPC_SecurityCamera::InputRagdoll( inputdata_t &inputdata )
{
	Ragdoll();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SecurityCamera::DeathThink( void )
{
	if ( PreThink( CAMERA_DEAD ) )
		return;

	// Level out our angles
	m_vecGoalAngles.x = 120.0f;
	m_vecGoalAngles.y = 0.0f;
	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( m_lifeState != LIFE_DEAD )
	{
		m_lifeState = LIFE_DEAD;

		//EmitSound( "NPC_CeilingTurret.Die" );
	}

	// lots of smoke
	Vector pos;
	CollisionProp()->RandomPointInBounds( vec3_origin, Vector( 1, 1, 1 ), &pos );
	
	CBroadcastRecipientFilter filter;
	
	te->Smoke( filter, 0.0, &pos, g_sModelIndexSmoke, 2.5, 10 );
	
	g_pEffects->Sparks( pos );

	if ( !UpdateFacing() )
	{
		m_flPlaybackRate = 0;
		SetThink( NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_SecurityCamera::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
	// If we're out of ammo, make friendly companions ignore us
	if ( m_spawnflags & SF_SECURITY_CAMERA_OUT_OF_AMMO )
	{
		if ( pEnemy->Classify() == CLASS_PLAYER_ALLY_VITAL )
			return false;
	} 

	return BaseClass::CanBeAnEnemyOf( pEnemy );
}

void CNPC_SecurityCamera::DormantThink()
{
	// Doesn't seem like any new camera state was added for this
	// and it gets optimized out anyway so I just picked the one that wasn't used yet
	if ( PreThink( CAMERA_AUTO_SEARCHING ) )
		return;

	//Update our think time
	SetNextThink( gpGlobals->curtime + 0.1f );
}

bool CNPC_SecurityCamera::CheckRestingSurfaceForPortals()
{
	if (!m_bEnabled)
		return false;

	// Check up to bounding radius (about 27 units with the P2 model)
	float flRadius = m_Collision.BoundingRadius();
	Vector vStartPoint = GetAbsOrigin() + Forward() * flRadius;
	Vector vEndPoint = GetAbsOrigin() - Forward() * flRadius;

	Ray_t ray;
	ray.Init(vStartPoint, vEndPoint);
	ray.m_IsRay = true;

	float thenumberone = 1.0f;
	CPortal_Base2D* pPortal = UTIL_Portal_FirstAlongRay(ray, thenumberone);
	if (pPortal)
	{
		if (pPortal->IsActivedAndLinked())
		{
			Ragdoll();
			return true;
		}
		pPortal->AddPortalEventListener(this);
	}
	return false;
}

void CNPC_SecurityCamera::NotifyPortalEvent(PortalEvent_t nEventType, CPortal_Base2D* pNotifier)
{
	if (nEventType == PORTALEVENT_LINKED)
		CheckRestingSurfaceForPortals();
}

void CNPC_SecurityCamera::FireGameEvent(IGameEvent* event)
{
	const char* name = event->GetName();
	if (!V_strcmp( name, "portal_player_ping" ))
	{
		int iUserID = event->GetInt("userid");
		Vector vecPingPos(event->GetFloat("ping_x"), event->GetFloat("ping_y"), event->GetFloat("ping_z"));

		int TeamNumber = -1;
		for (int i = 1; i <= MAX_PLAYERS; ++i)
		{
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
			if (pPlayer && pPlayer->GetUserID() == iUserID)
			{
				TeamNumber = pPlayer->GetTeamNumber();
				break;
			}
		}

		// 1 means any here
		if (m_nTeamToLookAt == 1 || m_nTeamToLookAt == TeamNumber)
		{
			m_vecPingLocation = vecPingPos;
			m_bDetectedNewPing = true;
		}
	}
}

void CNPC_SecurityCamera::TauntedByPlayer(CPortal_Player* pPlayer)
{
	if (pPlayer)
	{
		m_OnTaunted.FireOutput(pPlayer, pPlayer, 0.0f);

		if (pPlayer->GetTeamNumber() == TEAM_BLUE)
			m_OnTauntedBlue.FireOutput(pPlayer, pPlayer, 0.0f);
		else if (pPlayer->GetTeamNumber() == TEAM_RED)
			m_OnTauntedOrange.FireOutput(pPlayer, pPlayer, 0.0f);
	}
}

void CNPC_SecurityCamera::TauntedByPlayerFinished(CPortal_Player* pPlayer)
{
	if (pPlayer)
	{
		m_OnTauntedFinished.FireOutput(pPlayer, pPlayer, 0.0f);

		if (pPlayer->GetTeamNumber() == TEAM_BLUE)
			m_OnTauntedBlueFinished.FireOutput(pPlayer, pPlayer, 0.0f);
		else if (pPlayer->GetTeamNumber() == TEAM_RED)
			m_OnTauntedOrangeFinished.FireOutput(pPlayer, pPlayer, 0.0f);
	}
}

// As far as I can tell this was actually duplicated like this, no evidence of an inlined function
void CNPC_SecurityCamera::InputLookAtBlue( inputdata_t &inputdata )
{
	ClearEnemyMemory();
	SetEnemy( NULL );
	SetLastSightTime();
	SetThink( &CNPC_SecurityCamera::SearchThink );

	m_nTeamPlayerToLookAt = TEAM_BLUE;
}

void CNPC_SecurityCamera::InputLookAtOrange( inputdata_t &inputdata )
{
	ClearEnemyMemory();
	SetEnemy( NULL );
	SetLastSightTime();
	SetThink( &CNPC_SecurityCamera::SearchThink );

	m_nTeamPlayerToLookAt = TEAM_RED;
}

void CNPC_SecurityCamera::InputLookAllTeams( inputdata_t &inputdata )
{
	ClearEnemyMemory();
	SetEnemy( NULL );
	SetLastSightTime();
	SetThink( &CNPC_SecurityCamera::SearchThink );

	// 0 means any here
	m_nTeamPlayerToLookAt = 0;
}