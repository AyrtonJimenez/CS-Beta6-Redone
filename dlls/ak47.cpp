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
	shoot3
};


class CAK47: public CBasePlayerWeapon
{
	public:
		void Spawn (void);
		void Precache (void);
		int iItemSlot(void) { return 2; }
		int GetItemInfo(ItemInfo *p);
		int AddToPlayer( CBasePlayer *pPlayer );
		void Reload(void);
		void PrimaryAttack(void);
		void AK47Fire(int timeSinceLastAttack);
		BOOL Deploy(void);
		void Holster(void);

};


LINK_ENTITY_TO_CLASS (weapon_ak47, CAK47);


void CAK47::Spawn() 
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_ak47.mdl");
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
	p->iSlot = 1;
	p->iPosition = 4;
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
	SendWeaponAnim(idle1);
}


void CAK47::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
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
		m_flNextPrimaryAttack = gpGlobals->time + 0.115;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->SetAnimation ( PLAYER_ATTACK1);

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_762, 0);

	// Trace Line Testing
	TraceResult tr;
	//Vector vecDir = gpGlobals->v_forward;
	//UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192, ignore_monsters, m_pPlayer->edict(), &tr);

	// if (tr.pHit->v.takedamage)
	// {
	// 	ALERT(at_console, "Something was hit");
	// }

	m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.115;
	if (m_flNextPrimaryAttack < gpGlobals->time)
		m_flNextPrimaryAttack = gpGlobals->time + 0.115;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	AK47Fire(gpGlobals->time);
}



void CAK47::AK47Fire(int timeSinceLastAttack)
{
	float multiplier = 1.0;
	int consecutiveShots = 0;

	if(timeSinceLastAttack <= 0.115 && consecutiveShots < 15)
	{
		m_pPlayer->pev->punchangle.x += multiplier;
		m_pPlayer->pev->punchangle.y -= 0.725;
		multiplier *= 2.5;
		consecutiveShots += 1;
	}
	if(timeSinceLastAttack <= 0.115 && consecutiveShots > 15)
	{
		m_pPlayer->pev->punchangle.x = 7.25;
		m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(0, 2);
		consecutiveShots += 1;
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= 1.5;
		m_pPlayer->pev->punchangle.y -= 0.725;
	}
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