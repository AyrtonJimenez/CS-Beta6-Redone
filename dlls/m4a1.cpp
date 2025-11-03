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
		int iItemSlot(void) { return WEAPON_PRIMARY; }
		int GetItemInfo(ItemInfo *p);
		int AddToPlayer(CBasePlayer *pPlayer);
		void Reload(void);
		void WeaponIdle(void);

		void PrimaryAttack(void);
		void SecondaryAttack(void);
		// void M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
		void M4A1Fire(float flSpread, float flCycleTime);
		float GetMaxSpeed(void);

		BOOL Deploy(void);
		void Holster(void);
		float flSpread;
		bool m_fInZoom;
		int m_iShell;
};

LINK_ENTITY_TO_CLASS (weapon_m4a1, CM4A1);

void CM4A1::Spawn() 
{
	m_iShotsFired = 0;
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

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

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
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M4A1;
	p->iWeight = M4A1_WEIGHT;
	return 1;
}

void CM4A1::WeaponIdle(void) {
	if(m_iShotsFired != 0) // Crucial piece of code to make the recoil work correctly!!!!
		m_iShotsFired--;

	// m_pPlayer->SetAnimation(PLAYER_IDLE);
}

int CM4A1::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		if(pPlayer->hasPrimary) {
			// ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "The Player already has a Primary Weapon");
			return false;
		}
		pPlayer->hasPrimary = true;
		// ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "hasPrimary == True");
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
			WRITE_BYTE(m_iId);
		MESSAGE_END();


		return true;
	}
	return false;
}


BOOL CM4A1::Deploy()
{
	strcpy( m_pPlayer->m_szAnimExtention, "silenced_rifle" );
	return DefaultDeploy("models/v_m4a1_r.mdl", "models/p_m4a1.mdl", DRAW, "m4a1");
}

void CM4A1::Holster()
{
	if(m_fInZoom)
	{
		m_pPlayer->m_iFOV = 0;
		m_fInZoom = !m_fInZoom;
	}
		
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5; //Same for all
	SendWeaponAnim(IDLE1);
}

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
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "dryfire_rifle.wav", 0.8, ATTN_NORM);
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	m_iClip--;

	strcpy( m_pPlayer->m_szAnimExtention, "silenced_rifle" );

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(SHOOT + RANDOM_LONG(0, 2));
	if(RANDOM_LONG(1,3) >= 2)
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1-2.wav", 1.0f, ATTN_NORM);
	else 
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1-1.wav", 1.0f, ATTN_NORM);


	Vector	vecShellVelocity = m_pPlayer->pev->velocity 
							 + gpGlobals->v_right * RANDOM_FLOAT(50,70) 
							 + gpGlobals->v_up * RANDOM_FLOAT(100,150) 
							 + gpGlobals->v_forward * 25;
	EjectBrass ( pev->origin + m_pPlayer->pev->view_ofs
					+ gpGlobals->v_up * -12 
					+ gpGlobals->v_forward * 20 
					+ gpGlobals->v_right * 4, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL); 

	if(m_fInZoom)
	{
		flSpread = 0.875f;
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.165f;
		if (m_flNextPrimaryAttack < gpGlobals->time)
			m_flNextPrimaryAttack = gpGlobals->time + 0.165f;
	}
	else
	{
		flSpread = 0.165f;
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.115;
		if (m_flNextPrimaryAttack < gpGlobals->time)
			m_flNextPrimaryAttack = gpGlobals->time + 0.115;
	}

	// if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	// 	// HEV suit - indicate out of ammo condition
	// 	m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	float cycTime;

	if(m_fInZoom)
		cycTime = 0.165f;
	else
		cycTime = 0.0875f;

	M4A1Fire(flSpread, cycTime);
}

void CM4A1::M4A1Fire(float flSpread, float flCycleTime) {
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

	if(m_flAccuracy > 1)
		m_flAccuracy = 1;

	// This needs to be here otherwise the engine won't add our two vectors
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);


	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;

	if (m_pPlayer->pev->speed <= 0.0f)
		if (m_fInZoom) // Zoomed in
			if (FL_DUCKING) // Crouched
				KickBack(0.55f, 0.3f, 0.2f, 0.0125f, 4.5f, 1.5f, 10);
			else
				KickBack(0.6f, 0.35f, 0.25f, 0.015f, 4.5f, 1.5f, 10);
		else // Not zoomed in
			KickBack(0.6f, 0.35f, 0.25f, 0.015f, 4.5f, 1.5f, 10);
	else // Player is moving
		KickBack(1.0f, 0.4f, 0.23f, 0.15f, 5.0f, 3.0f, 7);
	
	// m_pPlayer->FireBullets3(0, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 33);

	// m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 1, BULLET_PLAYER_556, 33);
	m_pPlayer->FireBullets( 0, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_556, 33);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
}

void CM4A1::SecondaryAttack(void)
 {
	if (m_fInZoom) {
		m_pPlayer->m_iFOV = 0;
		m_fInZoom = !m_fInZoom;
	}
	else if(!m_fInZoom) {
		m_pPlayer->m_iFOV = 45;
		m_fInZoom = !m_fInZoom;
	}
	m_flNextSecondaryAttack = gpGlobals->time + 0.5;
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
		m_fInZoom = false;
	}

	// EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1_clipout.wav", 0.8, ATTN_NORM);
	DefaultReload(M4A1_MAX_CLIP, RELOAD, 2.3);

	// EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1_clipin.wav", 0.8, ATTN_NORM);
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
