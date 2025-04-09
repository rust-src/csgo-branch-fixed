#include "cbase.h"
#include "c_baseanimating.h"
#include "materialsystem/IMaterialProxy.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterial.h"
#include "proxyentity.h"
#include "c_portal_player.h"

class C_Photograph : public C_BaseAnimating {
public:
    DECLARE_CLASS(C_Photograph, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
	const char* GetTextureName();

private:
	char m_szTextureName[260];
};

IMPLEMENT_CLIENTCLASS_DT(C_Photograph, DT_Photograph, CPhotograph)
	RecvPropString(RECVINFO(m_szTextureName)),
END_RECV_TABLE()

LINK_ENTITY_TO_CLASS(item_photograph, C_Photograph);

const char *C_Photograph::GetTextureName()
{
	return m_szTextureName;
}

class CPhotoMaterialProxy : public CEntityMaterialProxy 
{
public:
	CPhotoMaterialProxy(){};
	virtual ~CPhotoMaterialProxy(){};
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(C_BaseEntity *pC_BaseEntity);
	virtual IMaterial *GetMaterial();
private:
	IMaterialVar* m_BaseTextureVar;
};


bool CPhotoMaterialProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
    bool found = false;
    m_BaseTextureVar = pMaterial->FindVar("$basetexture", &found, false);
    return found;
}

IMaterial* CPhotoMaterialProxy::GetMaterial()
{
	IMaterial *result;

	if (m_BaseTextureVar)
	{

		result = m_BaseTextureVar->GetOwningMaterial();

		return result;
	}
	return NULL;
}

void CPhotoMaterialProxy::OnBind(C_BaseEntity *pC_BaseEntity)
{
    if(!pC_BaseEntity)
        return;

    C_Photograph *pPhoto = dynamic_cast<C_Photograph *>(pC_BaseEntity);
    if(!pPhoto)
        return;

    ITexture *pTexture = materials->FindTexture(pPhoto->GetTextureName(), TEXTURE_GROUP_MODEL, true);
    if(!pTexture)
        return;

    m_BaseTextureVar->SetTextureValue(pTexture);
}
EXPOSE_INTERFACE( CPhotoMaterialProxy, CEntityMaterialProxy, "PhotoMaterial" IMATERIAL_PROXY_INTERFACE_VERSION );


class CPlacementPhotoMaterialProxy : public CEntityMaterialProxy 
{
	IMaterial* m_pMaterial;
	IMaterialVar* m_pAlphaVar;

	bool m_bLastState;
	float m_flStartAlpha;
	float m_flTargetAlpha;
	float m_flFadeStartTime;
	float m_flFadeDuration;

	virtual void OnBind( C_BaseEntity * pC_BaseEntity );
	virtual bool Init( IMaterial * pMaterial, KeyValues * pKeyValues );
	virtual IMaterial * GetMaterial();
	double GetAlphaFade();
};

bool CPlacementPhotoMaterialProxy::Init(IMaterial * pMaterial, KeyValues * pKeyValues)
{
	bool found = false;
	m_pMaterial = pMaterial;
	m_pAlphaVar = pMaterial->FindVar( "$alpha", &found, true );
	if (!found) 
	{
		m_pAlphaVar = 0;
		return false;
	}
	else 
	{
		pMaterial->SetMaterialVarFlag( MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY, true );
		m_bLastState = false;
		return true;
	}

}

IMaterial * CPlacementPhotoMaterialProxy::GetMaterial()
{
	return m_pMaterial;
}

void CPlacementPhotoMaterialProxy::OnBind(C_BaseEntity * pC_BaseEntity)
{
	C_Portal_Player *LocalPlayer = dynamic_cast<C_Portal_Player * >(C_BasePlayer::GetLocalPlayer());
	if ( LocalPlayer )
	{
		if ( m_bLastState != LocalPlayer->GetPortalPlayerLocalData().m_bPlacingPhoto)//If m_bPlacingPhoto changed
		{
			m_flStartAlpha = GetAlphaFade();
			m_flFadeStartTime = gpGlobals->curtime;
			m_flFadeDuration = 0.4f;
			if (m_bLastState)
			{
				m_flTargetAlpha = 1.0f;
			}
			else
			{
				m_flTargetAlpha = 0.0;
			}
			m_bLastState = LocalPlayer->GetPortalPlayerLocalData().m_bPlacingPhoto;//Update the last state.
		}
		m_flStartAlpha = GetAlphaFade();
		m_pAlphaVar->SetFloatValue(m_flStartAlpha);
	}
}

double CPlacementPhotoMaterialProxy::GetAlphaFade()
{
	float fadeEndTime = m_flFadeDuration + m_flFadeStartTime;
	if ( m_flFadeStartTime == fadeEndTime )
	{
		if (gpGlobals->curtime < fadeEndTime)
			return m_flStartAlpha;
		else
			return m_flTargetAlpha;
	}
	else
	{
		float v7 = (gpGlobals->curtime - m_flFadeStartTime) / (fadeEndTime - m_flFadeStartTime);
		float v8 = 0.0;
		if ( v7 >= 0.0 )
		{
			v8 = 1.0f;
			if ( v7 <= 1.0 )
				v8 = v7;
		}
		return ( ( ( ( ( v8 * v8 ) * 3.0 ) - ( ( ( v8 * v8 ) * 2.0 ) * v8 ) ) * ( m_flTargetAlpha - m_flStartAlpha ) ) + m_flStartAlpha );
	}
}

EXPOSE_INTERFACE( CPlacementPhotoMaterialProxy, CEntityMaterialProxy, "PlacementPhoto" IMATERIAL_PROXY_INTERFACE_VERSION );