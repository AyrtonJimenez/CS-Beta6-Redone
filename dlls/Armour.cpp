#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

class CArmoury : public CBaseEntity {
    void KeyValue(KeyValue *pkvd);
    void ArmouryTouch(CBaseEntity *pOther);
    void Precahce(void);
    void Restart(void);
    void Spawn(void);
};

void CArmoury::KeyValue(KeyValue * pkvd) {
    return;
}

void CArmoury::ArmouryTouch(CBaseEntity *pOther) {
    CBasePlayer *pPlayer;
    if(pOther->IsPlayer())
        pPlayer = (CBasePlayer *) pOther;

    // TODO: Put the Switch statement here so that it'll use the right model for weapons
    // switch()
    
    return;
}

void CArmoury::Precache(void) {
    return;
}

void CArmoury::Spawn(void) {
    Precahce();
}