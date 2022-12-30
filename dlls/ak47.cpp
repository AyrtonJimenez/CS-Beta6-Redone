#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"






enum ak47_e 
{
	idle1,
	reload,
	draw,
	shoot1,
	shoot2,
	shoot3,
};


class CAK47: public CBasePlayerWeapon
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
};


LINK_ENTITY_TO_CLASS (weapon_ak47, CAK47);



void CAK47::Spawn() 
{
	Precache( ); //call precache
	m_iId = WEAPON_AK47; //this will be the name defined in weapons.h
	SET_MODEL(ENT(pev), "models/w_ak47.mdl"); //fill in your weapons model
	m_iClip = AK47_DEFAULT_GIVE; 

	FallInit();
}


void CAK47::Precache(void)
{
	PRECACHE_MODEL("models/w_ak47.mdl");
	PRECACHE_MODEL("models/v_ak47_r.mdl");
	PRECACHE_MODEL("models/p_ak47.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("sound/weapons/ak47-1.wav");
	PRECACHE_SOUND("sound/weapons/ak47-2.wav");
	PRECACHE_SOUND("sound/dryfire_rifle.wav");
	PRECACHE_SOUND("sound/weapons/ammo_pick.wav");
	PRECACHE_SOUND("sound/weapons/ak47_boltpull.wav");
	PRECACHE_SOUND("sound/weapons/ak47_clipin.wav");
	PRECACHE_SOUND("sound/weapons/ak47_clipout.wav");

}

int CAK47::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "7.62";
	p->iMaxAmmo1 = AK47_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AK47;
	p->iWeight = AK47_WEIGHT;
	return 1;
}


int CAK47::AddToPlayer( CBasePlayer *pPlayer )
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


BOOL CAK47::Deploy ()
{
	return DefaultDeploy( "models/v_ak47_r.mdl", "models/p_ak47.mdl", draw, "AK47");
}

void CAK47::Holster()
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5; //Same for all
	SendWeaponAnim( idle1);
}

void CAK47::WeaponIdle(void)
{
	SendWeaponAnim(idle1);
}

void CAK47::PrimaryAttack(void)
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
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sound/weapons/ak47-1.wav", 0.8, ATTN_NORM);
		m_flNextPrimaryAttack = gpGlobals->time + 0.1;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->SetAnimation ( PLAYER_ATTACK1);

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_762, 0);

	m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.02;
	if (m_flNextPrimaryAttack < gpGlobals->time)
		m_flNextPrimaryAttack = gpGlobals->time + 0.02;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	m_pPlayer->pev->punchangle.x -= 2;
}

void CAK47::SecondaryAttack(void)
{

}

void CAK47::Reload(void)
{
	DefaultReload(AK47_MAX_CLIP, reload, 1.5);
}

class CAK47Ammo: public CBasePlayerAmmo
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
		if (pOther->GiveAmmo(AMMO_AK47_GIVE, "7.62", AK47_MAX_CARRY)!= -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "sound/ammo_pick.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_ak47, CAK47Ammo);