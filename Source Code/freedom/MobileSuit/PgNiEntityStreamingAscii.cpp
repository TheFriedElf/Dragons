#include "StdAfx.h"
#include "NiFactories.h"
#include "PgNiFile.h"
#include "PgNiEntityStreamingAscii.h"

#include "NiRoomComponent.H"
#include "NiPortalComponent.H"
#include "NiPlaneComponent.H"
#include "NiBoxComponent.H"



#define PG_USE_PACK_IN_NIENTITYSTREAMING

PgNiEntityStreamingAscii::PgNiEntityStreamingAscii(void)
{
}

PgNiEntityStreamingAscii::~PgNiEntityStreamingAscii(void)
{
}

void PgNiEntityStreamingAscii::_SDMInit()
{
    STREAMING_EXTENSION = "PACK_GSA";
    STREAMING_DESCRIPTION = "Gamebryo ASCII Scene files (*.gsa)|*.gsa";

	NiRoomComponent::_SDMInit();
	NiPortalComponent::_SDMInit();
	NiPlaneComponent::_SDMInit();
	NiBoxComponent::_SDMInit();
}

void PgNiEntityStreamingAscii::_SDMShutdown()
{
    STREAMING_EXTENSION = NULL;
    STREAMING_DESCRIPTION = NULL;

	NiRoomComponent::_SDMShutdown();
	NiPortalComponent::_SDMShutdown();
	NiPlaneComponent::_SDMShutdown();
	NiBoxComponent::_SDMShutdown();

}

NiBool PgNiEntityStreamingAscii::Load(char const* pcFileName)
{
#ifdef PG_USE_PACK_IN_NIENTITYSTREAMING
	if (g_bUsePackData && g_bUseExtraPackData)
	{
		Flush();
		if (!PgStoreWorkingPath(pcFileName))
			return false;
		RemoveAllScenes();

		//-- Initialize
		{
			TiXmlDocument kXmlDoc(pcFileName);
			
			{
				std::wstring wstrPackFileName;
				std::wstring wstrChildName;
				PgNiFile::IsPackedFile(pcFileName, wstrPackFileName, wstrChildName);

				std::vector< char > data(0);
				if(!BM::PgDataPackManager::LoadFromPack(wstrPackFileName, wstrChildName, data))
				{
					NILOG(PGLOG_ERROR, "[PgXmlLoader] LoadFromPack failed, %s xml parse failed\n",pcFileName);
					return false;
				}
				
				if(!data.size())
				{
					PG_ASSERT_LOG(!"failed loading from Packed Data.");
					return false;
				}

				data.push_back('\0');

				kXmlDoc.Parse(&data.at(0));

				if(kXmlDoc.Error())
				{
					PG_ASSERT_LOG(!"failed to loading path.xml");
					return false;
				}
			}

		
			m_kDOM.Init(kXmlDoc);
		}

		/*
		//-- Read from disk
		if (!m_kDOM.LoadFile())
		{
			//if (m_kDOM.HasError())
			//{
			//    char acString[2048];
			//    sprintf(acString,"Error ID: %d\nError Desciption: %s\n" 
			//       "Row: %d\nCol:%d", m_kDOM.GetErrorID(), m_kDOM.GetErrorDesc(),
			//        m_kDOM.GetErrorRow(), m_kDOM.GetErrorCol());
			//}       
			Flush();

			ReportError(ERR_FILE_LOAD_FAILED, pcFileName,"", "");
			return false;
		}
		*/

		//-- Read from DOM (and create a Scene)
		if (!ReadFromDOM())
		{   
			Flush();
			RemoveAllScenes();

			ReportError(ERR_FILE_PARSE_FAILED, pcFileName, "", "");
			return false;
		}

		Flush();
		return true;
	}
	else
#endif
	{
		return NiEntityStreamingAscii::Load(pcFileName);
	}

	return false;
}

NiBool PgNiEntityStreamingAscii::PgStoreWorkingPath(char const* pcFileName)
{
	NiStrcpy(m_acFullPath, NI_MAX_PATH, pcFileName);

	// Strip off the filename
	char* pcLastSlash = strrchr(m_acFullPath, NI_PATH_DELIMITER_CHAR);
	if (pcLastSlash == NULL)
	{
		pcLastSlash = strrchr(m_acFullPath, NI_PATH_DELIMITER_INCORRECT_CHAR);
	}

	if (pcLastSlash)
		*pcLastSlash = '\0';

	return true;
}
