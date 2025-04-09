#include "cbase.h"


class CAI_AddonFactory : public CPointEntity,
	public IEntityListener
{
    void InputDisable(inputdata_t &);
    void InputEnable(inputdata_t&);
    bool m_bDisabled;
};

void CAI_AddonFactory::InputDisable(inputdata_t &)
{
    if (!m_bDisabled) 
    {
        gEntList.RemoveListenerEntity(this);
        m_bDisabled = true;
    }
}

void CAI_AddonFactory::InputEnable(inputdata_t &)
{
    if (m_bDisabled) 
    {
        m_bDisabled = false;
        //FUN_1800790e0();
    }
}