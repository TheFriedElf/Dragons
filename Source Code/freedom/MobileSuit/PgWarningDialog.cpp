#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuestFullScreenDialog.h"
#include "PgWarningDialog.h"
#include "PgUIScene.h"
#include "PgUIModel.h"
#include "lwUI.h"
#include "lwUIQuest.h"


extern XUI::CXUI_Wnd* CallQuestWnd(std::wstring const &rkFormName);

PgWarningDialog::PgWarningDialog()
{
}

PgWarningDialog::~PgWarningDialog()
{
}

bool PgWarningDialog::CallWarningDialog(const SFullScreenDialogInfo& rkInfo)
{
	XUI::CXUI_Wnd* pkQuestWnd = CallQuestWnd(_T("FRM_QUEST_WARNING"));
	if( !pkQuestWnd )
	{
		return false;
	}
	pkQuestWnd->Visible(true);

	//const ContQuestDialogText::value_type& rkDialogText = (*pkQuestDialog->kDialogText.begin());

	XUI::CXUI_Wnd* pkTextWnd = pkQuestWnd->GetControl(_T("FRM_TEXT"));
	XUI::CXUI_Wnd* pkModelWnd = pkQuestWnd->GetControl(_T("FRM_MODEL"));

	//Text
	if( pkTextWnd )
	{
		std::wstring kText = rkInfo.kDialogText;
		Quest::lwQuestTextParser(kText);
		pkTextWnd->Text(kText);
	}

	//Face
	if( pkModelWnd )
	{
		char const *szWarningUIModelName = "QuestWarning";
		char const *szWarningActorName = "WarningActor";
		PgUIModel* pkUIModel = g_kUIScene.FindUIModel(szWarningUIModelName);
		if( !pkUIModel )
		{
			return false;
		}

		if( pkUIModel->AddActorByID(szWarningActorName, MB(rkInfo.kFaceID)) )
		{
			if( rkInfo.iFaceAniID )
			{
				pkUIModel->SetTargetAnimation(szWarningActorName, rkInfo.iFaceAniID);
			}

			NiCamera *pkCamera = pkUIModel->GetCamera();
			if( pkCamera
			&&	POINT3() != rkInfo.kCamPos )
			{
				pkCamera->SetTranslate(rkInfo.kCamPos.x, rkInfo.kCamPos.y, rkInfo.kCamPos.z);
				pkCamera->Update(0.f);
			}
			else
			{
				NiActorManager* pkAM = pkUIModel->GetActor(szWarningActorName);
				if( pkAM )
				{
					NiAVObject* pkRoot = pkAM->GetNIFRoot();
					if( pkRoot )
					{
						pkRoot->SetTranslate(NiPoint3(0.f, 0.f, -25.f));
					}
				}
			}
		}
	}

	return true;
}