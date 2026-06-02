#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERERUTIL_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERERUTIL_H
#include <NiMain.H>

namespace PgRendererUtil
{
	template	<typename T>
	void	RemoveParticleSystemModifierRecursive(NiAVObject *pkAVObject)
	{
		if(!pkAVObject)
		{
			return;
		}

		NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int	const	iTotalChild = pkNode->GetArrayCount();
			for(int i=0;i<iTotalChild;++i)
			{
				RemoveParticleSystemModifierRecursive<T>(pkNode->GetAt(i));
			}
		}

		NiParticleSystem	*pkParticleSystem = NiDynamicCast(NiParticleSystem,pkAVObject);
		if(pkParticleSystem)
		{
			RemoveParticleSystemModifier<T>(pkParticleSystem);
		}
	}

	template	<typename T>
	void	RemoveParticleSystemModifier(NiParticleSystem *pkParticleSystem)
	{
		T	*pkModifier = FindParticleSystemModifier<T>(pkParticleSystem);
		if(pkModifier)
		{
			pkParticleSystem->RemoveModifier(pkModifier);
		}

	}

	template	<typename T>
	T*	FindParticleSystemModifier(NiParticleSystem *pkParticleSystem)
	{
		if(!pkParticleSystem)
		{
			return NULL;
		}

		int	iNumModifier = pkParticleSystem->GetModifierCount();

		for(int i=0;i<iNumModifier;++i)
		{
			NiPSysModifier	*pkModifier = pkParticleSystem->GetModifierAt(i);

			T	*pkTModifier = NiDynamicCast(T,pkModifier);
			if(pkTModifier)
			{
				return pkTModifier;
			}
		}

		return	NULL;
	}

	template	<typename T>
	void	RemoveController(NiAVObject *pkAVObject)
	{
		NiTimeController	*pkController = pkAVObject->GetControllers();
		while(pkController)
		{
			T	*pkTController = NiDynamicCast(T,pkController);
			if(pkTController)
			{
				pkAVObject->RemoveController(pkTController);
				return;
			}

			pkController = pkController->GetNext();
		}
	}

	template	<typename T>
	T*	FindObjectByType(NiAVObject *pkAVObject)
	{
		if(!pkAVObject)
		{
			return	NULL;
		}

		T*	pkObj = NiDynamicCast(T,pkAVObject);
		if(pkObj)
		{
			return	pkObj;
		}

		NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int	const	iTotalChild = pkNode->GetArrayCount();
			for(int i=0;i<iTotalChild;++i)
			{
				pkObj = FindObjectByType<T>(pkNode->GetAt(i));
				if(pkObj)
				{
					return	pkObj;
				}
			}			
		}

		return	NULL;
	}
};
#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERERUTIL_H