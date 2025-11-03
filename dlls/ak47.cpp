#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


enum AK47_e 
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
		int iItemSlot(void) { return WEAPON_PRIMARY; }
		int GetItemInfo(ItemInfo *p);
		int AddToPlayer( CBasePlayer *pPlayer );
		void Reload(void);
		void PrimaryAttack(void);
		void AK47Fire(float flSpread, float fCycleTime);
		float GetMaxSpeed(void);
		BOOL Deploy(void);
		void Holster(void);


		private:
		float flSpread;
};


LINK_ENTITY_TO_CLASS (weapon_ak47, CAK47);


void CAK47::Spawn() 
{
	Precache();
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), "models/w_ak47.mdl");
	m_iClip = AK47_DEFAULT_GIVE; 
	m_tGunType = WEAPON_PRIMARY;

	FallInit();
}

void CAK47::Precache(void)
{
	PRECACHE_MODEL("models/w_ak47.mdl");
	PRECACHE_MODEL("models/v_ak47_r.mdl");
	PRECACHE_MODEL("models/p_ak47.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/ak47-1.wav");
	PRECACHE_SOUND("weapons/ak47-2.wav");
	PRECACHE_SOUND("dryfire_rifle.wav");
	PRECACHE_SOUND("weapons/ammo_pick.wav");
	PRECACHE_SOUND("weapons/ak47_boltpull.wav");
	PRECACHE_SOUND("weapons/ak47_clipin.wav");
	PRECACHE_SOUND("weapons/ak47_clipout.wav");
}


int CAK47::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "7.62";
	p->iMaxAmmo1 = AK47_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 1; //The horizontal one
	p->iPosition = 1; //The vertical one
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AK47;
	p->iWeight = AK47_WEIGHT;
	return 1;
}


int CAK47::AddToPlayer( CBasePlayer *pPlayer )
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


BOOL CAK47::Deploy ()
{
	return DefaultDeploy("models/v_ak47_r.mdl", "models/p_ak47.mdl", draw, "AK47");
}

void CAK47::Holster()
{
	if(m_iShotsFired > 0)
		m_iShotsFired--;
	// m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5; //Same for all
	// SendWeaponAnim(idle1);
}


void CAK47::PrimaryAttack(void)
{
	if(m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if(m_iClip <= 0)
	{
		if(m_fFireOnEmpty)
			if(!m_pPlayer->m_afButtonReleased)
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "dryfire_rifle.wav", 0.8, ATTN_NORM);
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	m_iClip--;

	int rand = (int) RANDOM_FLOAT(1, 3);

	SendWeaponAnim(shoot1 + RANDOM_LONG(0, 2));
	if(rand >= 2)
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak47-2.wav", 1.0f, ATTN_NORM);
	else 
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak47-1.wav", 1.0f, ATTN_NORM);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// flSpread = 0.165f;
	// m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.115;
	// if (m_flNextPrimaryAttack < gpGlobals->time)
	// 	m_flNextPrimaryAttack = gpGlobals->time + 0.115;



	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	if (!FL_ONGROUND)
		AK47Fire(0.04 + (0.4) * m_flAccuracy, 0.0955);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		AK47Fire(0.04 + (0.07) * m_flAccuracy, 0.0955);
	else
		AK47Fire((0.0275), 0.0955);
		

	// AK47Fire(flSpread, 0.0955f);
}

void CAK47::AK47Fire(float flSpread, float flCycleTime) {
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

	if(m_flAccuracy > 0.65f)
		m_flAccuracy = 0.65f;

	// This needs to be here otherwise the engine won't add our two vectors
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);


	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;

    if (m_pPlayer->pev->speed <= 0.0f) // Standing still
    {
        if (m_pPlayer->pev->flags & FL_ONGROUND)
        {
            if (m_pPlayer->pev->flags & FL_DUCKING)
            {
                // Crouched + grounded
                KickBack(0.9f, 0.35f, 0.15f, 0.025f, 5.5f, 1.5f, 9);
            }
            else
            {
                // Standing + grounded
                KickBack(1.0f, 0.375f, 0.175f, 0.0375f, 5.75f, 1.75f, 8);
            }
        }
        else
        {
            // In air, stationary
            KickBack(2.0f, 1.0f, 0.5f, 0.35f, 9.0f, 6.0f, 5);
        }
    }
    else // Moving
    {
        KickBack(1.5f, 0.45f, 0.25f, 0.185f, 7.0f, 4.0f, 6);
    }

	// if (m_pPlayer->pev->speed <= 0.0f)
	// 	if (FL_DUCKING) // Crouched
	// 		KickBack(0.55f, 0.3f, 0.2f, 0.0125f, 4.5f, 1.5f, 10);
	// 	else // Not zoomed in
	// 		KickBack(0.6f, 0.35f, 0.25f, 0.015f, 4.5f, 1.5f, 10);
	// else // Player is moving
	// 	KickBack(1.0f, 0.4f, 0.23f, 0.15f, 5.0f, 3.0f, 7);
	
	// m_pPlayer->FireBullets3(0, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 33);

	m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 1, BULLET_PLAYER_762, 33);
	// m_pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 0);

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8f;
}

float CAK47::GetMaxSpeed(void) { return 221.0f; }

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