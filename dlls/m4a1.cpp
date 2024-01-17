#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"






enum m4a1_e {

draw,
shoot,
burst_shoot,
reload,
idle1

};


class CM4A1: public CBasePlayerWeapon
{
	public:
		void Spawn (void);
		void Precache (void);
		int iItemSlot( void ) { return 1; }
		int GetItemInfo(ItemInfo *p);
		int AddToPlayer( CBasePlayer *pPlayer );
		void Reload(void);

		void PrimaryAttack( void );
		void SecondaryAttack(void);

		BOOL Deploy( void );
		void Holster( void );
		void WeaponIdle( void );


		int m_fInZoom;

};


LINK_ENTITY_TO_CLASS (weapon_m4a1, CM4A1);



void CM4A1::Spawn() 
{
	Precache( ); //call precache
	m_iId = WEAPON_M4A1; //this will be the name defined in weapons.h
	SET_MODEL(ENT(pev), "models/w_m4a1.mdl"); //fill in your weapons model
	m_iClip = M4A1_DEFAULT_GIVE; 

	FallInit();
}


void CM4A1::Precache(void)
{
	PRECACHE_MODEL("models/w_m4a1.mdl");
	PRECACHE_MODEL("models/v_m4a1_r.mdl");
	PRECACHE_MODEL("models/p_m4a1.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("sound/weapons/m4a1-1.wav");
	PRECACHE_SOUND("sound/weapons/m4a1-2.wav");
	PRECACHE_SOUND("sound/dryfire_rifle.wav");
	PRECACHE_SOUND("sound/weapons/ammo_pick.wav");
	PRECACHE_SOUND("sound/weapons/m4a1_deploy.wav");
	PRECACHE_SOUND("sound/weapons/m4a1_clipin.wav");
	PRECACHE_SOUND("sound/weapons/m4a1_clipout.wav");

}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "5.56";
	p->iMaxAmmo1 = M4A1_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M4A1_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M4A1;
	p->iWeight = M4A1_WEIGHT;
	return 1;
}


int CM4A1::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


BOOL CM4A1::Deploy ()
{
	return DefaultDeploy( "models/v_m4a1_r.mdl", "models/p_m4a1.mdl", draw, "M4A1");
}

void CM4A1::Holster()
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5; //Same for all
	SendWeaponAnim( idle1);
}

void CM4A1::WeaponIdle(void)
{
	SendWeaponAnim(idle1);
}

void CM4A1::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if(!m_fFireOnEmpty)
			return;
		else
		{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sound/weapons/m4a1-1.wav", 0.8, ATTN_NORM);
		m_flNextPrimaryAttack = gpGlobals->time + 0.1;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->SetAnimation ( PLAYER_ATTACK1);

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 0);

	m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.01;
	if (m_flNextPrimaryAttack < gpGlobals->time)
		m_flNextPrimaryAttack = gpGlobals->time + 0.02;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	m_pPlayer->pev->punchangle.x -= 2;
}

void CM4A1::SecondaryAttack(void)
{
	if (m_fInZoom)
	{
		m_pPlayer->m_iFOV = 0;
		m_fInZoom = 0;
	}
	else 
	{
		m_pPlayer->m_iFOV = 45;
		m_fInZoom = 1;
	}
	pev->nextthink = gpGlobals->time + 0.1;
	m_flNextSecondaryAttack = gpGlobals->time + 0.375;
}

void CM4A1::Reload(void)
{
	DefaultReload(M4A1_MAX_CLIP, reload, 1.5);
}

class CM4A1Ammo: public CBasePlayerAmmo
{
	void Spawn (void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_weaponbox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache (void)
	{
		PRECACHE_MODEL("models/w_weaponbox.mdl");
		PRECACHE_SOUND("sound/ammo_pick.wav");
	}
	BOOL AddAmmo(CBaseEntity * pOther)

	{
		if (pOther->GiveAmmo(AMMO_M4A1_GIVE, "5.56", M4A1_MAX_CARRY)!= -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "sound/ammo_pick.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_m4a1, CM4A1Ammo);
