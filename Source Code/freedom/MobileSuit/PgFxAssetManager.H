#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXASSETMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXASSETMANAGER_H

#include "FxReference/FxAssetManager.h"

class	PgFxAssetManager	:	public	FxAssetManager
{
public:
	struct CaselessLess
	{
		bool operator()(const std::string& lhs, const std::string& rhs) const
		{
			return NiStricmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};

public:

	virtual NiSourceTexturePtr GetSourceTexture(char const* szFileName);
	virtual NiFont*			 GetFont(char const* szFileName);

	// Be sure to always clone the objects from this stream.
	virtual NiStream*		 GetStream(char const* szFileName);

	virtual	NiNodePtr	GetNif(char const* szFileName);

	// Release the object on the next frame, used by the components to delay release a resource which must last for as long
	// as the next call to the renderer.
	virtual void ReleaseNextFrame(NiObject* pkObject);

	void	ReleaseFrameDelayedAssets();

private:

	std::string ConvertMediaFileName(std::string const &kFileName);

private:

	typedef std::map< std::string, NiFontPtr, CaselessLess >		  FontMap;

	FontMap			 m_Fonts;

	std::vector<NiObjectPtr>	m_ReleaseQueue;
};

NiSmartPointer(PgFxAssetManager);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXASSETMANAGER_H