#ifndef FREEDOM_DRAGONICA_PGNODEMNG_H
#define FREEDOM_DRAGONICA_PGNODEMNG_H

#include "PgWorld.h"
class PgRenderer;
class PgInput;

template< typename _T_Renderer, typename _T_Node >
class PgNodeRenderer
{
public:
	PgNodeRenderer( _T_Renderer* pkRender, NiCamera* pkCamera, float fFrameTime )
		:m_pkRender(pkRender), m_pkCamera(pkCamera), m_fFrameTime(fFrameTime)
	{}
	~PgNodeRenderer() {}

	bool operator ()(_T_Node* pkNode)
	{
		pkNode->DrawImmediate(m_pkRender, m_pkCamera, m_fFrameTime);
		return true;
	}

private:
	_T_Renderer* m_pkRender;
	NiCamera* m_pkCamera;
	float m_fFrameTime;
};

template< class _T_Node, class _T_Renderer = PgRenderer, class _T_Input = PgInput >
class PgNodeMgr
{
protected:
	typedef std::list< _T_Node* > ContNode;
	typedef BM::TObjectPool< _T_Node > ContPool;

private:
	template< class _T_Node >
	class PgNodeDeleter
	{
	public:
		PgNodeDeleter(ContPool& rkPool)
			:m_rkPool(rkPool)
		{}
		~PgNodeDeleter() {}

		bool operator () (_T_Node* pkNode)
		{
			m_rkPool.Delete(pkNode);
			return true;
		}

	private:
		ContPool& m_rkPool;
	};

public:
	PgNodeMgr()
		:m_kNodePool(10, 100)
	{
		Clear();
	}

	virtual ~PgNodeMgr() {Destroy();}

	_T_Node* CreateNode()
	{
		_T_Node* pkNewNode = m_kNodePool.New();
		PG_ASSERT_LOG(pkNewNode && __FUNCTIONW__);
		m_kNodeList.push_back(pkNewNode);
		return pkNewNode;
	}

	void DestroyNode(_T_Node *&pkNode)
	{
		if( !pkNode ) return;
		ContNode::iterator iter = std::find(m_kNodeList.begin(), m_kNodeList.end(), pkNode);
		if( m_kNodeList.end() != iter )
		{
			m_kNodePool.Delete((*iter));
			m_kNodeList.erase(iter);
		}
		else
		{
			PG_ASSERT_LOG(0 && pkNode && "Can't find this pointer");
		}
		pkNode = NULL;
	}

	virtual void DrawImmediate(_T_Renderer* pkRenderer, float fFrameTime)
	{
		if( !g_pkWorld || m_kNodeList.size() == 0) return;
		if( !g_pkWorld->GetCameraMan() ) return;
		NiCamera* pkCamera = g_pkWorld->GetCameraMan()->GetCamera();
		if( !pkCamera ) return;

		PgNodeRenderer< _T_Renderer, _T_Node > kRender(pkRenderer, pkCamera, fFrameTime);
		std::for_each(m_kNodeList.rbegin(), m_kNodeList.rend(), kRender);
	}

	virtual bool ProcessInput(_T_Input *pkInput)
	{
		return false;
	}

	virtual void Init()
	{}

	virtual void Clear()
	{
		if (m_kNodeList.size() > 0)
		{
			PgNodeDeleter< _T_Node > kWorker(m_kNodePool);
			std::for_each(m_kNodeList.begin(), m_kNodeList.end(), kWorker);
		}
		m_kNodeList.clear();
	}

	void Destroy()
	{
		Clear();
		m_kNodePool.Terminate();
	}

protected:
	ContNode m_kNodeList;

private:
	ContPool m_kNodePool;
};
#endif // FREEDOM_DRAGONICA_PGNODEMNG_H