#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "effects.h"

#include "ww_satchels.h"


LINK_ENTITY_TO_CLASS( ww_satchel, WWSatchel );


void WWSatchel::Spawn( void )
{
	Precache();
	// 1.2.5 Satchel spawn in things fix Ticket #4
//	SET_SIZE   ( edict(), Vector( -4, -4, 0 ), Vector( 4, 4, 8 ) );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	SET_ORIGIN ( edict(), pev->origin );

	pev->movetype		= MOVETYPE_BOUNCE;
	pev->solid			= SOLID_SLIDEBOX;
	pev->avelocity		= Vector( 90, 90, 90 );

	m_flExplodeDelay	= 0.0f;
	m_iExplodeCount		= 0;

	m_flActivateTime	= 0.0f;
	m_flExplodeTime		= 0.0f;

	pev->sequence = LookupSequence( "idle" );
	ResetSequenceInfo();

	SetTouch(&WWSatchel:: SatchelTouch );
	SetThink(&WWSatchel:: SatchelThink );
	pev->nextthink = gpGlobals->time + 0.1f;
}


void WWSatchel::Precache( void )
{
	PRECACHE_MODEL( "models/satchels/death.mdl" );
	PRECACHE_MODEL( "sprites/stmbal1.spr"		);
	PRECACHE_SOUND( "satchels/death.wav"		);
	m_usEvent = PRECACHE_EVENT( 1, "events/satchels/death.sc" );

	SET_MODEL( edict(), "models/satchels/death.mdl" );
}


void WWSatchel::SetExplode( float flTime )
{
	m_flActivateTime = flTime;
}


void WWSatchel::SatchelTouch( CBaseEntity * pOther )
{
	pev->velocity = pev->velocity * 0.3f;
}


void WWSatchel::SatchelThink( void )
{
	if( gpGlobals->time >= m_flActivateTime )
		SetThink(&WWSatchel:: ExplodeThink );

	if( pev->flags & FL_ONGROUND && pev->velocity.Length() < 32.0f )
		pev->angles = Vector( 0, 0, 0 );

	pev->nextthink = gpGlobals->time + 0.1f;
}


void WWSatchel::ExplodeThink( void )
{
	if( m_flExplodeTime == 0.0f )
	{
		pev->sequence = LookupSequence( "active" );
		ResetSequenceInfo();

		m_flExplodeTime = gpGlobals->time + m_flExplodeDelay;
	}

	CBaseEntity * pNoGrenades = NULL;
	while( ( pNoGrenades = UTIL_FindEntityByClassname( pNoGrenades, "func_nogrenades" ) ) != NULL )
	{
		if( pNoGrenades->ContainsPoint( pev->origin ) )
		{
			pev->effects |= EF_LIGHT;
			SetThink(&WWSatchel:: SUB_Remove );
			pev->nextthink = gpGlobals->time + 0.1f;
			return;
		}
	}

	if( pev->flags & FL_ONGROUND && pev->velocity.Length() < 32.0f )
		pev->angles = Vector( 0, 0, 0 );

	SatchelTick();

	if( gpGlobals->time >= m_flExplodeTime )
	{
		pev->sequence = LookupSequence( "explode" );
		ResetSequenceInfo();

		SatchelExplode();

		m_iExplodeCount--;
		m_flExplodeTime = gpGlobals->time + m_flMultiDelay;
	}

	if( m_iExplodeCount <= 0 )
		UTIL_Remove( this );

	pev->nextthink = gpGlobals->time + 0.1f;
}


void WWSatchel::SatchelTick( void )
{
}


void WWSatchel::SatchelExplode( void )
{
}