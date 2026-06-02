#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGNIFMAN_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGNIFMAN_H

#include "NiMain.h"
#include "CreateUsingNiNew.inl"

typedef struct tagNifScene
{
	NiNodePtr spNifNode;
	NiPhysXScenePtr spPhysXScene;

	tagNifScene()
	{
		spNifNode = 0;
		spPhysXScene = 0;
	}
	~tagNifScene()
	{
		spNifNode = 0;
		spPhysXScene = 0;
	}
} SNifScene;

class	PgMobileSuit;

//	NIF 파일을 로딩 및 관리를 해준다.
class	PgNifMan	
{
public:
	PgNifMan();
	virtual	~PgNifMan();

	void	Init();
	void	Destroy();

	NiPhysXScenePtr GetPhysX(const std::string& NifFilePath);
	NiNodePtr GetNif(const std::string& NifFilePath, NiObjectNET::CopyType kCopyType = NiObjectNET::COPY_EXACT);	//	Nif 파일을 로딩한 후, 클론 한 오브젝트를 리턴한다.
	NiNodePtr GetNif_DeepCopy(const std::string& NifFilePath);	//	Nif 파일을 로딩한 후, 클론 한 오브젝트를 리턴한다.(이때에는 DeepCopy를 사용한다)
	SNifScene GetNewNifScene(const std::string& NifFilePath, NiObjectNET::CopyType kCopyType = NiObjectNET::COPY_EXACT);
	bool DeleteAllNif();	//	컨테이너 클리어

	NiSourceTexturePtr GetTexture(const std::string& TexturePath);
	bool DeleteAllTexture();	//	컨테이너 클리어
protected:
	NiNodePtr findNif(const std::string& NifFilePath);
	NiSourceTexturePtr findTexture(const std::string& NifFilePath);	

	friend struct Loki::CreateUsingNew< PgNifMan >;

	typedef std::map<std::string, NiNodePtr> NifContainer;
	typedef std::map<std::string, NiSourceTexturePtr> TextureContainer;
	
	NifContainer m_NifContainer;
	Loki::Mutex m_kNifLock;	
	
	TextureContainer m_TextureContainer;
	Loki::Mutex m_kTextureLock;	
};

#define g_kNifMan SINGLETON_CUSTOM(PgNifMan, Loki::CreateUsingNew)

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGNIFMAN_H