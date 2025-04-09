#include "cbase.h"
#include "material_index_data_ops_proxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

MaterialIndexDataOpsProxy s_MaterialIndexDataOpsProxy;

MaterialIndexDataOpsProxy& GetMaterialIndexDataOpsProxy()
{
	return s_MaterialIndexDataOpsProxy;
}

bool MaterialIndexDataOpsProxy::IsEmpty(const SaveRestoreFieldInfo_t& fieldInfo)
{
    ISaveRestoreOps* piVar2 = physprops->GetMaterialIndexDataOps();
    return piVar2->IsEmpty(fieldInfo);
}

void MaterialIndexDataOpsProxy::Save(const SaveRestoreFieldInfo_t& fieldInfo, ISave* pSave)
{
    ISaveRestoreOps* piVar2 = physprops->GetMaterialIndexDataOps();
    return piVar2->Save(fieldInfo,pSave);
}

void MaterialIndexDataOpsProxy::Restore(const SaveRestoreFieldInfo_t& fieldInfo, IRestore * pRestore)
{
    ISaveRestoreOps* piVar2 = physprops->GetMaterialIndexDataOps();
    return piVar2->Restore(fieldInfo,pRestore);
}

void MaterialIndexDataOpsProxy::MakeEmpty(const SaveRestoreFieldInfo_t& fieldInfo)
{
    ISaveRestoreOps* piVar2 = physprops->GetMaterialIndexDataOps();
    return piVar2->MakeEmpty(fieldInfo);
}

bool MaterialIndexDataOpsProxy::Parse(const SaveRestoreFieldInfo_t& fieldInfo, char const* szValue)
{
    ISaveRestoreOps* piVar2 = physprops->GetMaterialIndexDataOps();
    return piVar2->Parse(fieldInfo,szValue);
}