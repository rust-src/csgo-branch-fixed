#include "baseentity.h"
#include "ai_initutils.h"
#include "ai_dynamiclink.h"

struct storer;

class CFunc_Pipe_Entrance : public CBaseEntity
{
public:
	DECLARE_CLASS(CFunc_Pipe_Entrance, CBaseEntity);
	DECLARE_DATADESC();

	CFunc_Pipe_Entrance();
	~CFunc_Pipe_Entrance();

	void Spawn();
	void Activate();
	void InputToggle(inputdata_t &);
	void InputEnable(inputdata_t &);
	void InputDisable(inputdata_t &);
	void UpdateOnRemove();

	void FUN_1800a7080();

	virtual void Enable();
	virtual void Disable();
	virtual void Toggle();

	float m_fFlowRate;
	float m_fFlowTimeStamp;

	bool field_0x538;
	CUtlVector<storer *> field16_0x550;

	int m_iIndexInArray;

	string_t m_PipeSystemName;
};

struct storer
{
	CFunc_Pipe_Entrance* pPipe;
	CAI_DynamicLink* pLink;
};

extern CUtlVector<CFunc_Pipe_Entrance*> m_PipeEntrances;