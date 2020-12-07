#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"
#include "saverestore.h"
#include "monsters.h"
#include "player.h"

#include "ww_seals.h"


class DeathSeal : public WWSeal
{
public:
	virtual void	Spawn		( void );
	virtual void	Precache	( void );

	virtual void	Process		( CBaseEntity * pOther );

private:
	unsigned short	m_usEvent;
};


LINK_ENTITY_TO_CLASS( ww_seal_death, DeathSeal );


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DeathSeal::Spawn( void )
{
	Precache();
	WWSeal::Spawn();

	SET_MODEL( ENT(pev), "models/seals/death.mdl" );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DeathSeal::Precache( void )
{
	PRECACHE_MODEL( "models/seals/death.mdl" );
	PRECACHE_MODEL( "sprites/flare1.spr"	);
	PRECACHE_SOUND( "seals/death.wav"		);
	m_usEvent = PRECACHE_EVENT( 1, "events/seals/death.sc" );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DeathSeal::Process( CBaseEntity * pOther )
{
	if( !pOther || !pOther->IsPlayer() || !pOther->IsAlive() )
		return;

	CBasePlayer * pPlayer = ((CBasePlayer *)pOther);

	if( pPlayer->IRelationship( this ) < R_NO )
	{
		if( m_flMana < 20.0f )
			return;

		if( pPlayer->IsTimerActive( TIMER_INVULNERABILITY ) )
			return;

		pPlayer->SetTimer( TIMER_INVULNERABILITY, 5.0f );

		m_flMana -= 20.0f;
		m_flShowTime = gpGlobals->time + 5.0f;
		m_flNextProcess = gpGlobals->time + 1.0f;

		PLAYBACK_EVENT_SHORT( FEV_HOSTONLY | FEV_GLOBAL, pev->owner, m_usEvent,
			m_flMana, 0, 0, pPlayer->entindex(), 1, 0 );
		PLAYBACK_EVENT_SHORT( 0, edict(), m_usEvent, 0, 0, 1, pPlayer->entindex(), 0, 0 );
	}
	else
	{
		if( m_flMana < 10.0f )
			return;

		pPlayer->TakeDamage( pev, VARS( pev->owner ), 20, DMG_POISON );

		m_flMana -= 10.0f;
		m_flShowTime = gpGlobals->time + 5.0f;
		m_flNextProcess = gpGlobals->time + 0.5f;

		PLAYBACK_EVENT_SHORT( FEV_HOSTONLY | FEV_GLOBAL, pev->owner, m_usEvent,
			m_flMana, 0, 0, pPlayer->entindex(), 2, 0 );
		PLAYBACK_EVENT_SHORT( 0, edict(), m_usEvent, 0, 0, 2, pPlayer->entindex(), 0, 0 );
	}
}

