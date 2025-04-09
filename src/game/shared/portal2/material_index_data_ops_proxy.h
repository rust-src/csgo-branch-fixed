
class MaterialIndexDataOpsProxy : public CDefSaveRestoreOps
{
public:
		virtual void Save(const SaveRestoreFieldInfo_t& fieldInfo, ISave* pSave);
		virtual void Restore(const SaveRestoreFieldInfo_t& fieldInfo, IRestore* pRestore);

		virtual bool IsEmpty(const SaveRestoreFieldInfo_t& fieldInfo);
		virtual void MakeEmpty(const SaveRestoreFieldInfo_t& fieldInfo);
		virtual bool Parse(const SaveRestoreFieldInfo_t& fieldInfo, char const* szValue);
};


MaterialIndexDataOpsProxy& GetMaterialIndexDataOpsProxy();