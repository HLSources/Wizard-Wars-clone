#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"
#include "saverestore.h"
#include "weapons.h"
#include "nodes.h"
#include "doors.h"
#include "game.h"
#include "gamerules.h"
#include "items.h"
#include "player.h"

#include "tf_globals.h"
#include "tf_globalinfo.h"
#include "tf_teamcheck.h"
#include "tf_ent.h"
#include "tf_info_goal.h"
#include "tf_item_goal.h"
#include "tf_info_timer.h"
#include "tf_info_teamspawn.h"


LINK_ENTITY_TO_TF( info_player_teamspawn, TFInfoTeamSpawn, TFEntity );
LINK_ENTITY_TO_TF( i_p_t, TFInfoTeamSpawn, TFEntity );


TFInfoTeamSpawn::TFInfoTeamSpawn()
{
	m_bHasSpawned = FALSE;
}


TFInfoTeamSpawn::~TFInfoTeamSpawn( void )
{
}


//------------------------------------------------------------------------------
// Spawn:
//
//------------------------------------------------------------------------------
void TFInfoTeamSpawn::Spawn( void )
{
	Precache();

	m_iState = m_iInitialState;

	g_pTFGlobalInfo->AddGlobalClass( pev->playerclass );
	g_pTFGlobalInfo->AddGlobalTeam( m_iReqTeam );
}


//------------------------------------------------------------------------------
// Precache:
//
//------------------------------------------------------------------------------
void TFInfoTeamSpawn::Precache()
{
}


//------------------------------------------------------------------------------
// KeyValue:
//	Parsing settings
//------------------------------------------------------------------------------
void TFInfoTeamSpawn::KeyValue( KeyValueData * pkvd )
{	
	if( ParseSettings( pkvd->szKeyName, pkvd->szValue) )
		pkvd->fHandled = TRUE;
	else
	{
		ALERT( at_console, "TFInfoTeamSpawn::KeyValue( \"%s\", \"%s\" ) not handled.\n",
			pkvd->szKeyName, pkvd->szValue );

		CPointEntity::KeyValue( pkvd );
	}
};


//------------------------------------------------------------------------------
// ParseSettings:
//	Parsing settings
//------------------------------------------------------------------------------
BOOL TFInfoTeamSpawn::ParseSettings( const char * key, const char * value )
{
	if( TFStrEq( 1, key, "goal_state" ) )
	{
		if( atoi( value ) != 0 )
			m_iState = TF_GOALSTATE_REMOVED;
		else
			m_iState = TF_GOALSTATE_INACTIVE;

		return TRUE;
	}
	return TFEntity::ParseSettings( key, value );
}


//------------------------------------------------------------------------------
// CheckSpawn:
//	Returns wheither player can spawn here.
//------------------------------------------------------------------------------
BOOL TFInfoTeamSpawn::CheckSpawn( CBasePlayer * pPlayer )
{
	// if this is a bad ent
	if( !pPlayer || FNullEnt( pPlayer ) )
	{
		return FALSE;
	}

	// check if this class is allowed
	if( pev->playerclass != 0 && pPlayer->pev->playerclass != pev->playerclass )
	{
		return FALSE;
	}

	if( m_iTeamCheck != 0 )
	{
		TFTeamCheck * pTeamCheck = (TFTeamCheck *)UTIL_FindEntityByTargetname( NULL, STRING( m_iTeamCheck ) );
		
		if( !pTeamCheck )
		{
			return FALSE;
		}

		if( pPlayer->pev->team != pTeamCheck->GetTeam() )
		{
			return FALSE;
		}

		return TRUE;
	}

	if( m_iReqTeam != TF_TEAM_NONE && pPlayer->pev->team != m_iReqTeam )
	{
		return FALSE;
	}

	if( m_iState != TF_GOALSTATE_INACTIVE )
	{
		return FALSE;
	}

	return TRUE;
}


//------------------------------------------------------------------------------
// CheckSpawn:
//	Called when a player spawns at this spawn.
//------------------------------------------------------------------------------
void TFInfoTeamSpawn::PlayerSpawn( CBasePlayer * pPlayer )
{
	if( !pPlayer )
		return;

	CBaseEntity * pEntity = NULL;

	if( m_iOwnerTeamCheck != 0 )
	{
		TFTeamCheck * pTeamCheck = (TFTeamCheck *)UTIL_FindEntityByTargetname( NULL, STRING( m_iOwnerTeamCheck ) );

		if( pTeamCheck != NULL )
			m_iOwner = pTeamCheck->GetTeam();
	}

	if( m_iGoalResults & TF_TEAMSPAWN_AP_PASS_ACTIVATED_GOALS )
		pEntity = pPlayer;

	if( ( m_iGoalActivation & TF_TEAMSPAWN_ACTIVATION_ALL_GET_ITEM ) || !m_bHasSpawned )
	{
		if( CheckActivation( pEntity ) )
			ProcessGoals( pEntity );
		else
		{
			if( m_iGoalActivation & TF_TEAMSPAWN_ACTIVATION_ALL_GET_MESSAGE )
				PlayerMessage( pEntity, pPlayer );
		}
	}
	else
	{
		if( m_iGoalActivation & TF_TEAMSPAWN_ACTIVATION_ALL_GET_MESSAGE )
			PlayerMessage( pEntity, pPlayer );
	}

	// are we suppose to remove ourselves after a player spawns here?
	if( m_iGoalEffects & TF_TEAMSPAWN_EFFECTS_REMOVE_AFTER_SPAWN )
		GoalRemove( pPlayer );

	m_bHasSpawned = TRUE;
}


/* *** CDragonriderSpawn *** */

#include "ww_dragon.h"

class WWDragonriderSpawn: public TFInfoTeamSpawn{
public:
	void Precache(){
		TFInfoTeamSpawn::Precache();
	}

	void PlayerSpawn( CBasePlayer * pPlayer )
	{
//		TFInfoTeamSpawn::PlayerSpawn( pPlayer );

		if( !pPlayer )
			return;

		WWDragon *pDragon;
		
		pDragon=(WWDragon*)Create("monster_dragon",pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict());
		pDragon->pev->team		= pPlayer->pev->team;
		pDragon->pev->iuser1	= FALSE;
		pDragon->pev->iuser2	= TRUE;
		pDragon->Mount(pPlayer);
	}
/*
	void Spawn(CBaseEntity *pPlayer)
	{	
		ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "TEST");

	//	TFInfoTeamSpawn::PlayerSpawn((CBasePlayer*)pPlayer);

	//	if(pPlayer->IsPlayer() && pPlayer->IsAlive()){
			SetThink(GiveDragon);
			m_pPlayer=(CBasePlayer*)pPlayer;
			pev->nextthink=gpGlobals->time+.1;
	//	}
	}

	void EXPORT GiveDragon(){
		if( m_pPlayer->IsPlayer() && m_pPlayer->IsAlive() && 
			m_pPlayer->m_pClass && !m_pPlayer->m_pClass->m_bOnDragon)
		{
			WWDragon *pDragon;
			
			pDragon=(WWDragon*)Create("monster_dragon",m_pPlayer->pev->origin,m_pPlayer->pev->angles,edict());
			pDragon->pev->team=pev->team;
			pDragon->pev->iuser1=FALSE;
			pDragon->pev->iuser2=TRUE;
			pDragon->Mount(m_pPlayer);
		}
	}

	CBasePlayer* m_pPlayer;
*/
};

LINK_ENTITY_TO_CLASS(info_dragonrider_spawn, WWDragonriderSpawn);