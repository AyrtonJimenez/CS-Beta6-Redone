#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


enum M4A1_e {
	IDLE1,
	RELOAD,
	DRAW,
	SHOOT,
	SHOOT1,
	SHOOT2,
	BURST_SHOOT
};


class CM4A1: public CBasePlayerWeapon
{
	public:
		void Spawn (void);
		void Precache (void);
		int iItemSlot( void ) { return WEAPON_PRIMARY; }
		int GetItemInfo(ItemInfo *p);
		int AddToPlayer( CBasePlayer *pPlayer );
		void Reload(void);

		void PrimaryAttack( void );
		void SecondaryAttack(void);
		// void M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
		// void M4A1Fire(int timeSinceLastAttack);
		void M4A1Fire(void);
		float GetMaxSpeed(void);


		BOOL Deploy( void );
		void Holster( void );
		int m_fInZoom;

};


LINK_ENTITY_TO_CLASS (weapon_m4a1, CM4A1);

void CM4A1::Spawn() 
{
	Precache( ); //call precache
	m_iId = WEAPON_M4A1; //this will be the name defined in weapons.h
	SET_MODEL(ENT(pev), "models/w_m4a1.mdl"); //fill in your weapons model
	m_iClip = M4A1_DEFAULT_GIVE; 

	m_tGunType = WEAPON_PRIMARY;

	FallInit();
}


void CM4A1::Precache(void)
{
	PRECACHE_MODEL("models/w_m4a1.mdl");
	PRECACHE_MODEL("models/v_m4a1_r.mdl");
	PRECACHE_MODEL("models/p_m4a1.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/m4a1-1.wav");
	PRECACHE_SOUND("weapons/m4a1-2.wav");
	PRECACHE_SOUND("dryfire_rifle.wav");
	PRECACHE_SOUND("weapons/ammo_pick.wav");
	PRECACHE_SOUND("weapons/m4a1_deploy.wav");
	PRECACHE_SOUND("weapons/m4a1_clipin.wav");
	PRECACHE_SOUND("weapons/m4a1_clipout.wav");

	
}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "5.56";
	p->iMaxAmmo1 = M4A1_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M4A1_MAX_CLIP;
	p->iSlot = WEAPON_PRIMARY;
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
		if(pPlayer->hasPrimary) {
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "The Player already has a Primary Weapon");
			
			return FALSE;
		}
		pPlayer->hasPrimary = true;
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "hasPrimary == True");
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}


BOOL CM4A1::Deploy ()
{
	return DefaultDeploy("models/v_m4a1_r.mdl", "models/p_m4a1.mdl", DRAW, "m4a1");
}

void CM4A1::Holster()
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5; //Same for all
	SendWeaponAnim(IDLE1);
}

// void CM4A1::PrimaryAttack(void)
// {
// 	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
// 		M4A1Fire(0.035 + (0.4) * m_flAccuracy, 0.0875, FALSE);
// 	else if (m_pPlayer->pev->velocity.Length2D() > 140)
// 		M4A1Fire(0.035 + (0.07) * m_flAccuracy, 0.0875, FALSE);
// 	else
// 		M4A1Fire((0.025) * m_flAccuracy, 0.0875, FALSE);

// }

void CM4A1::PrimaryAttack(void)
{
		if(m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if(m_iClip <= 0)
	{
		if(!m_fFireOnEmpty)
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sound/weapons/dryfire_rifle.wav", 0.8, ATTN_NORM);
		else
		{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sound/weapons/m4a1-1.wav", 0.8, ATTN_NORM);
		m_flNextPrimaryAttack = gpGlobals->time + 0.1;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	m_iClip--;

	m_pPlayer->SetAnimation(PLAYER_SHOOT_SILENCED_RIFLE);

	if(m_fInZoom)
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.165;
		if (m_flNextPrimaryAttack < gpGlobals->time)
			m_flNextPrimaryAttack = gpGlobals->time + 0.165;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.115;
		if (m_flNextPrimaryAttack < gpGlobals->time)
			m_flNextPrimaryAttack = gpGlobals->time + 0.115;
	}

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}


	M4A1Fire();
}

void CM4A1::M4A1Fire() {
	if (m_pPlayer->pev->speed <= 0.0f)
	{
		if (m_fInZoom) // Zoomed in
		{ 
			if (FL_DUCKING) // Crouched
					KickBack(0.55f, 0.3f, 0.2f, 0.0125f, 4.5f, 1.5f, 10);
			else
				KickBack(0.6f, 0.35f, 0.25f, 0.15f, 4.5f, 1.5f, 10);
		}
		else
			KickBack(1.2f, 0.45f, 0.23f, 0.15f, 5.5f, 3.5f, 6);
		}
	else
	{
		// KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
		KickBack(1.0f, 0.4f, 0.23f, 0.15f, 5.0f, 3.0f, 7);
	}
	
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;
	m_pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 0);
}


// void CM4A1::M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
// {
// 	m_bDelayFire = true;
// 	m_iShotsFired++;
// 	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

// 	if (m_flAccuracy > 1)
// 		m_flAccuracy = 1;

// 	if (m_iClip <= 0)
// 	{
// 		if (m_fFireOnEmpty)
// 		{
// 			PlayEmptySound();
// 			m_flNextPrimaryAttack = gpGlobals->time + 0.2;
// 		}

// 		return;
// 	}

// 	m_iClip--;
// 	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

// 	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

// 	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
// 	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

// 	Vector vecSrc = m_pPlayer->GetGunPosition();
// 	Vector vecDir;

// 	vecDir = m_pPlayer->FireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556, 33, 0.95, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

// 	int flags;
// 	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

// 	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
// 		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

// 	m_flTimeWeaponIdle = gpGlobals->time + 1.5;

// 	if (m_pPlayer->pev->velocity.Length2D() > 0)
// 		KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
// 	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
// 		KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
// 	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
// 		KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
// 	else
// 		KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
// }

// void CM4A1::M4A1Fire(int timeSinceLastAttack)
// {
// 	float multiplier = 1.0;
// 	int consecutiveShots = 0;

// 	if(!m_fInZoom)
// 	{
// 		if(timeSinceLastAttack <= 0.115 && consecutiveShots < 15)
// 		{
// 			m_pPlayer->pev->punchangle.x += multiplier;
// 			m_pPlayer->pev->punchangle.y -= 0.725;
// 			multiplier *= 2.5;
// 			consecutiveShots += 1;
// 		}
// 		if(timeSinceLastAttack <= 0.115 && consecutiveShots > 15)
// 		{
// 			m_pPlayer->pev->punchangle.x = 7.25;
// 			m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(0, 2);
// 			consecutiveShots += 1;
// 		}
// 		else
// 		{
// 			m_pPlayer->pev->punchangle.x -= 1.5;
// 			m_pPlayer->pev->punchangle.y -= 0.725;
// 		}
// 	}
// 	else
// 	{
// 		if(timeSinceLastAttack <= 0.12 && consecutiveShots < 5)
// 		{
// 			m_pPlayer->pev->punchangle.x += multiplier;
// 			m_pPlayer->pev->punchangle.y -= 0.725;
// 			multiplier *= 1.75;
// 			consecutiveShots += 1;
// 		}
// 		if(timeSinceLastAttack <= 0.12 && consecutiveShots > 5)
// 		{
// 			m_pPlayer->pev->punchangle.x += multiplier;
// 			m_pPlayer->pev->punchangle.y += 0.725;
// 			multiplier *= 3.5;
// 			consecutiveShots += 1;
// 		}
// 		else
// 		{
// 			m_pPlayer->pev->punchangle.x -= 1.5;
// 			m_pPlayer->pev->punchangle.y -= 0.725;
// 		}
// 	}
// }

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
	m_flNextSecondaryAttack = gpGlobals->time + 1.0;
}

float CM4A1::GetMaxSpeed(void) 
{
	if (!m_fInZoom)
		return 230;
	else
		return 200;
}

void CM4A1::Reload(void)
{
	if(m_fInZoom)
	{
		m_pPlayer->m_iFOV = 0;
		m_fInZoom = 0;
	}
	DefaultReload(M4A1_MAX_CLIP, RELOAD, 1.5);
}

class CM4A1Ammo: public CBasePlayerAmmo
{
	void Spawn (void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache (void)
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity * pOther)

	{
		if (pOther->GiveAmmo(AMMO_M4A1_GIVE, "5.56", M4A1_MAX_CARRY)!= -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_m4a1, CM4A1Ammo);
