#include "matchmaking/iplayer.h"

class C_PortalMPStats
{
public:
	void RefreshStats(IPlayerLocal*, C_Portal_Player*);
};

C_PortalMPStats* GetPortalMPStats();