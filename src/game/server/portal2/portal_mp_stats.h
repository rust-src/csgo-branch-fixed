#include "portal_mp_gamerules.h"
class CPortalMPStats
{
public:
	void SetTimeToCompleteMap(float) {};
	void IncrementMapsCompleted() {};
	void SavePerMapStats(CPortal_Player*, const char*) {};
	void SubmitOGSEndOfMapStatsForPlayer(CPortal_Player*, const char*) {};
	void SaveStats(CPortal_Player*) {};
	void ClearPerMapStats() {};
	static void InitPortalMPStats() {};
	void IncrementPlayerPortalsTraveled(CPortal_Player*) {};
	void IncrementPlayerSteps(CPortal_Player*) {};
	void IncrementPlayerDeathsMap(CPortal_Player*) {};
	void SetStats(int, int, int, int) {};
	void IncrementPlayerTauntsUsedMap(CPortal_Player*, Coop_Taunts) {};
	void TeamTauntSuccess(const char*) {};
	void IncrementPlayerPortals(CPortal_Player*) {};
};
CPortalMPStats* GetPortalMPStats();
void ResetAllPlayersStats();