
class ICameraMode;

class CCameraMan : public NiMemObject
{
	typedef NiTStringPointerMap<NiCameraPtr> Container;

public:
	//// Constructor & Destructor
	//
	CCameraMan(void);
	~CCameraMan(void);

	//// Getter
	//
	NiCameraPtr GetCamera();
	void SetCamera(NiCameraPtr spCamera);
	void SetCamera(NiString& rkCameraName);

	//// Container
	//
	void AddCamera(NiString& rkCameraName, NiCameraPtr spCamera);
	void DeleteCamera(NiString& rkCameraName);

	//// Update
	//
	void Update(float fTime);

protected:
	NiCameraPtr m_spCamera;				// Current Camera
	Container m_kContainer;				// Camera Container
};