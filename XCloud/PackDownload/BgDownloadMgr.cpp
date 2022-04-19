#include "BgDownloadMgr.h"
#include <algorithm>
#include "pugixml.hpp"
#include "ReusePackMgr.h"
#include "PackLogSender.h"
#include "C3BaseFuncInternal.h"
#include "AppConfigInfo.h"

namespace c3pack_down
{
	CBgDownloadMgr* CBgDownloadMgr::s_pBgDownloadMgr = NULL;
	std::vector<std::string> m_vecPriority;

	CBgDownloadMgr::CBgDownloadMgr()
	{
		m_pResDownload = NULL;
		m_pResDownListen = NULL;
		m_pResDownItem = NULL;
		m_bPauseDownload = true;
		m_bStopDownload = false;
		m_nCurUndownIndex = 0;
		m_nFinishSize = 0;
		m_uTotalSize = 0;
		m_nFinishCount = 0;
		m_nTotalCount = 0;
		m_uDownLoadSpeed = 0;
		m_strCurDownFileName = "";
		m_strNextDownFile = "";
		m_strEnvPath = "";
		m_nBgThreadCreateNum = 1;
	}

	CBgDownloadMgr::~CBgDownloadMgr()
	{
		SAFE_DELETE(m_pResDownItem);
		SAFE_DELETE(m_pResDownListen);
	}

	CBgDownloadMgr* CBgDownloadMgr::GetInstance()
	{
		if (NULL == s_pBgDownloadMgr)
		{
			s_pBgDownloadMgr = new CBgDownloadMgr();
		}
		return s_pBgDownloadMgr;
	}

	void CBgDownloadMgr::ReleaseInstance()
	{
		CPackLogSender::ReleaseInstance();
		if (s_pBgDownloadMgr)
		{
			delete s_pBgDownloadMgr;
			s_pBgDownloadMgr = NULL;
		}
	}

	void CBgDownloadMgr::Init(IDownloadManage* pDownloadMgr, int nBgThreadCreateNum)
	{
		if (NULL == pDownloadMgr) return;
		m_pResDownload = pDownloadMgr->CreateTask();
		if (NULL == m_pResDownload) return;
		m_pVerify = pDownloadMgr->GetVertifyInstance();
		if (NULL == m_pVerify) return;
		m_pResDownItem = new CBgResDownloadItem();
		m_pResDownListen = new CBgResDownloadListen();

		m_nBgThreadCreateNum = nBgThreadCreateNum > 0 ? nBgThreadCreateNum : 1;
		bool bSuc = m_pResDownload->StartTask(m_pResDownItem, m_pResDownListen, m_nBgThreadCreateNum);
		if (!bSuc)
		{
			CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_CREATE_LIB_ERROR, "pack", nBgThreadCreateNum, true);
			return;
		}

		m_strEnvPath = c3pack_down::getAppCachePath();
	}

	void CBgDownloadMgr::ReLoad(int nStrategyType)
	{
		std::string  strFileFullPath = c3pack_down::getAppCachePath();
		strFileFullPath.append("/");
		strFileFullPath.append(DOWNLOAD_UPDATE_SORT);

		if (c3pack_down::IsFileExist(strFileFullPath.c_str()) && (m_vecUpdateListInfo.size() > 0) && (m_nTotalCount != m_nFinishCount))
		{//资源未下载完成时,做重新排序
			int nSaveStrategy = GetSaveStategy();
			if (nSaveStrategy != nStrategyType)
			{
				SortUpdateList(nStrategyType);
				SaveUpdateList(DOWNLOAD_UPDATE_SORT, nStrategyType);
			}
		}
		else
		{//该版本资源已经下载完成
			DebugMsg("res download over");
		}
	}

	bool CBgDownloadMgr::LoadPriorityConfig(int nStrategy)
	{
		m_vecPriority.clear();

		char szStrategy[256] = { 0 };
		sprintf(szStrategy, "Strategy_%d", nStrategy);

		MAP_PRIORITY mapPriority = CAppConfigInfo::GetInstance()->getMapPriority();
		if (mapPriority.size() > 0 )
		{
			MAP_PRIORITY::iterator iter = mapPriority.find(szStrategy);
			if (iter != mapPriority.end())
			{
				m_vecPriority = iter->second;
			}
		}
		return m_vecPriority.size() > 0 ? true : false;
	}

	bool CBgDownloadMgr::PriorityStrategy(const UpdateListInfo& info1, const UpdateListInfo& info2)
	{
		int nIndex1 = -1;
		int nIndex2 = -1;
		bool bContain1 = false;
		bool bContain2 = false;

		int nSize = m_vecPriority.size();
		for (int i = 0; i < nSize; ++i)
		{
			std::string& strPrior = m_vecPriority.at(i);
			int nLen = strPrior.length();
			if (0 == strncmp(info1.strPack.c_str(), strPrior.c_str(), nLen))
			{
				bContain1 = true;
				nIndex1 = i;
				break;
			}
		}

		for (int j = 0; j < nSize; ++j)
		{
			std::string& strPrior = m_vecPriority.at(j);
			int nLen = strPrior.length();
			if (0 == strncmp(info2.strPack.c_str(), strPrior.c_str(), nLen))
			{
				bContain2 = true;
				nIndex2 = j;
				break;
			}
		}

		if (bContain1 && !bContain2)
		{
			return true;
		}
		else if (!bContain1 && bContain2)
		{
			return false;
		}
		else if (bContain1 && bContain2)
		{
			if (nIndex1 < nIndex2)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	bool CBgDownloadMgr::LoadUpdateList(const char* pszUpdataList, bool bUpdate)
	{
		if (NULL == pszUpdataList) return false;
		//ZBTODO
	//	std::string  strEnvPath = DataSupport_GetRootPath();
	//	std::string  strExtendPath = DataSupport_GetExtendPath();
		std::string  strEnvPath = "";
		std::string  strExtendPath = "";

		std::string  strUpdateList = strEnvPath;
		strUpdateList.append(pszUpdataList);

		if (!c3pack_down::IsFileExist(strUpdateList.c_str())) return false;

		pugi::xml_document doc;
		if (!doc.load_file(strUpdateList.c_str()))
		{
			::LogMsg("open xml file %s fail", strUpdateList.c_str());
			return false;
		}

		pugi::xml_node rootNode = doc.root();
		pugi::xml_node nodePackage = rootNode.child("PackageInfo");


		m_vecUpdateListInfo.clear();
		m_mapFileNameIndex.clear();
		int nIndex = 0;

		m_uTotalSize = 0;
		m_nFinishSize = 0;
		m_nTotalCount = 0;
		m_nFinishCount = 0;
		int nStratege = -1;
		m_nCurUndownIndex = -1;

		//========================================================================
		m_nFinishCount = nodePackage.attribute("finish_count").as_int();
		std::string strValue = nodePackage.attribute("finish_size").as_string();
		m_nFinishSize = strtoul(strValue.c_str(), NULL, 10); // 超过4G会越界[未处理]
		m_uTotalSize = m_nFinishSize;
		//========================================================================
		pugi::xml_node nodePack = nodePackage.child("Pack");
		for (; !nodePack.empty(); nodePack=nodePack.next_sibling("Pack"))
		{
			bool bFileExist = false;
			bool bFileExclude = false;
			UpdateListInfo updateInfo;
			pugi::xml_node nodeFileSize = nodePack.child("FileSize");
			pugi::xml_node nodeVersion = nodePack.child("Version");
			pugi::xml_node nodeFileHash = nodePack.child("FileHash");
			pugi::xml_node nodeFileName = nodePack.child("FileName");

			updateInfo.uFileSize = nodeFileSize.text().as_uint();
			updateInfo.nVersion = nodeVersion.text().as_int();
			updateInfo.uFileHash = nodeFileHash.text().as_uint();

			m_uTotalSize += updateInfo.uFileSize;
			std::string strFileName = nodeFileName.text().as_string();
			if (IsDownloadExcludeFile(strFileName.c_str()))
			{
				bFileExclude = true;
				continue;
			}
			updateInfo.strPack = strFileName;
			std::string  strPackPath = strEnvPath;
			strPackPath.append(updateInfo.strPack.c_str());
			bFileExist = c3pack_down::IsFileExist(strPackPath.c_str());
			if (!bFileExist && !strExtendPath.empty())
			{
				strPackPath = strExtendPath;
				strPackPath.append(updateInfo.strPack.c_str());
				bFileExist = c3pack_down::IsFileExist(strPackPath.c_str());
				if (bFileExist)
				{
					DebugMsg("AppPack:%s %d", updateInfo.strPack.c_str(), updateInfo.uFileSize);
				}
			}
			if (bFileExist)
			{
				if (updateInfo.uFileSize == 0)
				{
					DebugMsg("Bg load update.xml error");
				}
				m_nFinishSize += updateInfo.uFileSize;
				m_nFinishCount += 1;
			}
			else
			{
				if (m_nCurUndownIndex == -1)
				{
					m_nCurUndownIndex = nIndex;
				}
			}

			if (!bFileExist && !bFileExclude)
			{
				m_vecUpdateListInfo.push_back(updateInfo);
				m_mapFileNameIndex[updateInfo.strPack] = nIndex;
				nIndex++;
			}
		}
		m_nTotalCount = m_vecUpdateListInfo.size() + m_nFinishCount;
		return true;
	}

	bool CBgDownloadMgr::SaveUpdateList(const char* pszRelName, int nStrategySave)
	{
		if (NULL == pszRelName) return false;
		
		pugi::xml_document doc;
		doc.reset();
		pugi::xml_node nodeDoc = doc.root();

		pugi::xml_node nodePackage = nodeDoc.append_child("PackageInfo");
		//===============================================================
		nodePackage.append_attribute("finish_count", m_nFinishCount);
		nodePackage.append_attribute("finish_size", (unsigned int)m_nFinishSize);
		//===============================================================
		if (m_vecUpdateListInfo.size() == 0) return false;
		UpdateListInfo_VEC::iterator iter = m_vecUpdateListInfo.begin();
		m_mapFileNameIndex.clear();
		m_nCurUndownIndex = -1;
		int nIndex = 0;
		//ZBTODO
	//	std::string  strEnvPath = DataSupport_GetRootPath();
	//	std::string  strExtendPath = DataSupport_GetExtendPath();
		std::string  strEnvPath = "";
		std::string  strExtendPath = "";

		for (; iter != m_vecUpdateListInfo.end(); iter++)
		{
			m_mapFileNameIndex[iter->strPack] = nIndex;
			if (iter->bHasDown)
			{
				nIndex++;
				continue;
			}
			pugi::xml_node nodePack = nodePackage.append_child("Pack");

			pugi::xml_node nodeFileSize = nodePack.append_child("FileSize");
			nodeFileSize.text().set((unsigned int)iter->uFileSize);


			pugi::xml_node nodeVersion = nodePack.append_child("Version");
			nodeVersion.text().set((int)iter->nVersion);

			pugi::xml_node nodeFileHash = nodePack.append_child("FileHash");
			nodeFileHash.text().set((unsigned int)iter->uFileHash);

			pugi::xml_node nodeFileName = nodePack.append_child("FileName");
			nodeFileName.text().set(iter->strPack.c_str());


			if (-1 == m_nCurUndownIndex)
			{
				std::string  strPackPath = strEnvPath;
				strPackPath.append(iter->strPack.c_str());
				bool bFileExist = c3pack_down::IsFileExist(strPackPath.c_str());
				if (!bFileExist && !strExtendPath.empty())
				{
					strPackPath = strExtendPath;
					strPackPath.append(strPackPath.c_str());
					bFileExist = c3pack_down::IsFileExist(strPackPath.c_str());
				}
				if (!bFileExist)
				{//对第一个未下载的包的索引重新赋值
					m_nCurUndownIndex = nIndex;
				}
			}
			nIndex++;
		}

		std::string  strFile = c3pack_down::getAppCachePath();
		strFile.append("/");
		strFile.append(pszRelName);

		if (c3pack_down::IsFileExist(strFile.c_str()))
		{
			c3pack_down::MyDeleteFile(strFile.c_str());
		}

		bool bSuc = doc.save_file(strFile.c_str());
		if (!bSuc) DebugMsg("xml update list save fail");

		if (nStrategySave >= 0)
		{
			SaveStategy(nStrategySave);
		}
		return bSuc;
	}

	void CBgDownloadMgr::SortUpdateList(int nStrategyType)
	{
		if (nStrategyType < 0) return;
		bool bLoad = LoadPriorityConfig(nStrategyType);
		if (bLoad)
		{
			std::sort(m_vecUpdateListInfo.begin(), m_vecUpdateListInfo.end(), CBgDownloadMgr::PriorityStrategy);
		}
	}

	void CBgDownloadMgr::ProcessBgDown()
	{
		if (NULL == m_pResDownload) return;

		m_pResDownload->EventProcess();
		EnPackCompleteStatus eStatus;
		const char* pFileName = m_pResDownload->PopDownloadItem(eStatus);
		if (pFileName)
		{
			if (eStatus == EnRes_Completed)
			{
				SetFinishInfo(GetUpdateInfo(pFileName));
				CheckDownloadFinish();
			}
			else
			{
				CPackLogSender::GetInstance()->NetDownLog(TQ_AUTOPATCH, "bg download file error filename=%s", pFileName);
			}
			std::set<std::string>::iterator iter = m_setDowningItem.find(pFileName);
			if (iter != m_setDowningItem.end())
			{
				m_setDowningItem.erase(iter);
			}
		}

		if (m_nFinishCount == m_nTotalCount) // 下载结束
		{
			return;
		}

		int nUnFinishItem = m_pResDownload->UnFinishDownloadItemCount();
		int nPushCount = m_nBgThreadCreateNum - nUnFinishItem;
		if (nPushCount > 0)
		{
			for (int i = 0; i <= nPushCount; ++i)
			{
				StartDownloadRes();
			}
		}
	}

	void CBgDownloadMgr::StartDownloadRes()
	{
		if (m_bPauseDownload) return;//暂停下载
		const char* pItem = GetUnDownLoadItem();
		if (pItem)
		{
			m_pResDownload->PushDownloadItem(false, pItem);
			m_setDowningItem.insert(pItem);
		}
	}

	void CBgDownloadMgr::PauseDownloadRes()
	{
		m_bPauseDownload = true;
	}

	void CBgDownloadMgr::ResumeDownloadRes()
	{
		m_bPauseDownload = false;
	}

	void CBgDownloadMgr::StopBgDownloadRes()
	{
		m_bStopDownload = true;
	}

	//前台询问后台是否在下载该资源
	bool CBgDownloadMgr::IsDownloadingRes(const char* pszRes)
	{
		if (NULL == pszRes) return false;
		std::string strRes = pszRes;
		std::set<std::string>::iterator iter = m_setDowningItem.find(strRes);
		if (iter != m_setDowningItem.end())
		{
			return true;
		}
		return false;
	}

	//后台正在下载的资源是否完成
	bool CBgDownloadMgr::IsDownloadItemOver(const char* pszRes)
	{
		if (NULL == pszRes) return false;
		std::string strRes = pszRes;
		std::string  strFile = m_strEnvPath.c_str();
		strFile.append("/");
		strFile.append(strRes.c_str());
		if (c3pack_down::IsFileExist(strFile.c_str()))
		{
			return true;
		}
		return false;
	}

	const char* CBgDownloadMgr::GetUnDownLoadItem()
	{
		int nSize = m_vecUpdateListInfo.size();
		if (nSize == 0) return NULL;
		m_strNextDownFile = "";
		while (m_nCurUndownIndex < nSize && m_nCurUndownIndex >= 0)
		{
			UpdateListInfo info = m_vecUpdateListInfo.at(m_nCurUndownIndex);
			m_nCurUndownIndex++;
			//检测update.xml里的文件，是否被前台已经下载了
			std::string  strFileFullPath = m_strEnvPath.c_str();
			strFileFullPath.append("/");
			strFileFullPath.append(info.strPack.c_str());
			if (c3pack_down::IsFileExist(strFileFullPath.c_str()))
			{
				//有可能被另一个进程下载,
				if (info.bHasDown == false)
				{
					m_nFinishCount += 1;
					m_nFinishSize += info.uFileSize;
					info.bHasDown = true;
					CPackLogSender::GetInstance()->SetPackDownInfo(m_nFinishCount, m_nTotalCount, info.strPack);
				}
				continue;
			}
			m_strNextDownFile = info.strPack;
			return m_strNextDownFile.c_str();
		};

		return NULL;
	}

	int CBgDownloadMgr::GetSaveStategy(void)
	{
		std::string  strPriorFile = m_strEnvPath.c_str();
		strPriorFile.append("/");
		strPriorFile.append(FILE_PRIOR);

		int nStrategy = 0;
		FILE* pPriorFile = fopen(strPriorFile.c_str(), "r");
		if (pPriorFile)
		{
			fscanf(pPriorFile, "%d", &nStrategy);
			SAFE_FCLOSE(pPriorFile);
		}
		return nStrategy;
	}

	void CBgDownloadMgr::SaveStategy(int nStrategy)
	{
		if (nStrategy >= 0)
		{
			std::string  strPriorFile = m_strEnvPath.c_str();
			strPriorFile.append("/");
			strPriorFile.append(FILE_PRIOR);

			FILE* pPriorFile = fopen(strPriorFile.c_str(), "w");
			if (pPriorFile)
			{
				fprintf(pPriorFile, "%d", nStrategy);
				SAFE_FCLOSE(pPriorFile);
			}
		}
	}

	void CBgDownloadMgr::SetUpdateFinishFlag(int nVersion)
	{
		std::string  strFlagFile = c3pack_down::getAppCachePath();
		strFlagFile.append("/");
		strFlagFile.append(UPDATE_FINISH_FLAG);

		FILE* pFile = fopen(strFlagFile.c_str(), "w");
		if (pFile)
		{
			fprintf(pFile, "%d", nVersion);
			SAFE_FCLOSE(pFile);
		}
	}

	bool CBgDownloadMgr::ResetUpdateFinishFlag(void)
	{
		std::string  strFlagFile = c3pack_down::getAppCachePath();
		strFlagFile.append("/");
		strFlagFile.append(UPDATE_FINISH_FLAG);

		bool bRet = false;
		if (c3pack_down::IsFileExist(strFlagFile.c_str()))
		{
			bRet = c3pack_down::MyDeleteFile(strFlagFile.c_str());
		}
		return bRet;
	}

	bool CBgDownloadMgr::IsExistUpdateFinishFlag()
	{
		std::string  strFlagFile = c3pack_down::getAppCachePath();
		strFlagFile.append("/");
		strFlagFile.append(UPDATE_FINISH_FLAG);

		bool bRet = false;
		if (c3pack_down::IsFileExist(strFlagFile.c_str()))
		{
			bRet = true;
		}
		return bRet;
	}

	unsigned int CBgDownloadMgr::GetFileSizeByName(const char* pszFileName)
	{
		if (NULL == pszFileName) return 0;
		std::string strFileName = pszFileName;
		if (strFileName.length() == 0) return 0;
		int nListInfoSize = m_vecUpdateListInfo.size();
		if (m_mapFileNameIndex.size() == 0 || nListInfoSize == 0) return 0;

		if (m_mapFileNameIndex.find(strFileName) != m_mapFileNameIndex.end())
		{
			int nFileIndex = m_mapFileNameIndex[strFileName];
			if (nFileIndex >= nListInfoSize) return 0;
			UpdateListInfo info = m_vecUpdateListInfo.at(nFileIndex);
			return info.uFileSize;
		}
		return 0;
	}

	CBgDownloadMgr::UpdateListInfo* CBgDownloadMgr::GetUpdateInfo(const char* pszFileName)
	{
		if (NULL == pszFileName) return 0;
		std::string strFileName = pszFileName;
		if (strFileName.length() == 0) return 0;
		int nListInfoSize = m_vecUpdateListInfo.size();
		if (m_mapFileNameIndex.size() == 0 || nListInfoSize == 0) return 0;

		if (m_mapFileNameIndex.find(strFileName) != m_mapFileNameIndex.end())
		{
			int nFileIndex = m_mapFileNameIndex[strFileName];
			if (nFileIndex >= nListInfoSize) return 0;
			UpdateListInfo& info = m_vecUpdateListInfo.at(nFileIndex);
			return &info;
		}
		return NULL;
	}

	void CBgDownloadMgr::SetFinishInfo(CBgDownloadMgr::UpdateListInfo* pInfo)
	{
		if (NULL == pInfo)
		{
			return;
		}
		m_nFinishCount += 1;
		m_nFinishSize += pInfo->uFileSize;
		m_strCurDownFileName = pInfo->strPack;
		pInfo->bHasDown = true;
		CPackLogSender::GetInstance()->SetPackDownInfo(m_nFinishCount, m_nTotalCount, m_strCurDownFileName);
	}

	bool CBgDownloadMgr::CheckDownloadFinish()
	{
		bool bRet = false;
		if (m_nFinishCount == m_nTotalCount)
		{//下载完成,删除更新列表 update_sort.xml
			std::string  strUpdateList = c3pack_down::getAppCachePath();
			strUpdateList.append("/");
			strUpdateList.append(DOWNLOAD_UPDATE_SORT);
			if (c3pack_down::IsFileExist(strUpdateList.c_str()))
			{
				bRet = c3pack_down::MyDeleteFile(strUpdateList.c_str());
			}
		}
		return bRet;
	}

	void CBgDownloadMgr::GetPackDownInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, char* pszCurFileName)
	{
		nTotalCount = m_nTotalCount;
		nFinishCount = m_nFinishCount > m_nTotalCount ? m_nTotalCount : m_nFinishCount;

		uTotalSize = m_uTotalSize / 1024;
		uDownSize = m_nFinishSize > m_uTotalSize ? (m_uTotalSize / 1024) : (m_nFinishSize / 1024);

		if (pszCurFileName)
		{
			pszCurFileName = (char*)m_strCurDownFileName.c_str();
		}
	}

	void CBgDownloadMgr::SetDownloadSpeed(int uSpeed)
	{
		m_uDownLoadSpeed = uSpeed;
	}

	int CBgDownloadMgr::GetDownloadSpeed(void)
	{
		return m_uDownLoadSpeed;
	}

	bool CBgDownloadMgr::CheckFreeSpaceLowerThan(int nCheckMb)
	{
		int nFreeSpace = c3pack_down::MyGetFreeSpace();
		if (nFreeSpace < nCheckMb)
		{
			return true;
		}
		return false;
	}

	void CBgDownloadMgr::SetExcludeFolder(const char* pszFolderNames)
	{
		if (NULL == pszFolderNames) return;
		std::string  strExcludeFolder = pszFolderNames;
		std::string strExcludeFolder2= "";
		for (int s = 0; s < strExcludeFolder.size(); ++s)
		{
			if (strExcludeFolder.at(s) == ' ')
			{
				continue;
			}
			strExcludeFolder2.push_back(strExcludeFolder.at(s));
		}

		if (strExcludeFolder2.size() > 0)
		{
			m_vecExcludeFolders.clear();
			SplitString(strExcludeFolder2, m_vecExcludeFolders, ",");
		}
		for (int i = 0; i < m_vecExcludeFolders.size();++i)
		{
			std::string strFolderName = m_vecExcludeFolders.at(i);
			std::string strSeperator = "/";
			std::string strFolder = strSeperator + strFolderName + strSeperator;
			m_vecExcludeFolders[i] = strFolder;
		}
	}


	bool CBgDownloadMgr::IsDownloadExcludeFile(const char* pszFileName)
	{
		int nSize = m_vecExcludeFolders.size();
		if (nSize == 0) return false;
		if (NULL == pszFileName || 0 == strlen(pszFileName)) return false;
		std::string strExcludeFolder = "";
		std::string strFileName = pszFileName;
		for (int i = 0; i < nSize; ++i)
		{
			strExcludeFolder = m_vecExcludeFolders.at(i);
			if (strFileName.find(strExcludeFolder) != std::string::npos)
			{
				return true;
			}
		}
		
		return false;
	}

	IPackDownloadTask* CBgDownloadMgr::GetDownloadTask(void)
	{
		return m_pResDownload;
	}

	// 下载项开始执行
	void CBgResDownloadItem::OnPackDownloadBegin(const char *pszFileName)
	{
		//::DebugMsg("\nBG res Download Begin...FileName=%s, time=%d", pszFileName, m_tmDownload);
	}

	// 下载项执行过程中的状态信息
	void CBgResDownloadItem::OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte)
	{
		//DebugMsg("NetDown: Downloading FileName=%s packSize=%d, hasDown=%d",pszFileName, nTotalByte, nCurrByte);
	}

	// 下载项完成
	void CBgResDownloadItem::OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr)
	{
		if (enStatus == EnRes_Completed)
		{
			int nAddr = (int)enAddr;
			if (pszFileName)
			{
				CPackLogSender::GetInstance()->NetDownLog("debug", "BG Download Addr=%d FileName=%s Version=%d", nAddr, pszFileName, nPackVersion);
				if (enAddr != EnAddr_Cdn)
				{
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_BACK_SRCIP_ERROR, pszFileName, nAddr, true);
				}
			}
		}
		else
		{
			if (pszFileName)
			{
				int nAddr = (int)enAddr;
				int nStatus = (int)enStatus;

				if (enAddr == EnAddr_BakIp) // 3个ip都失败时为最终失败
				{
					std::string strFileFullPath = CPackLogSender::GetInstance()->GetDownErrorFileName(nPackVersion, enAddr, pszFileName);
					if (enStatus >= EnRes_NotFount)
					{//如果手机空间小于10M,则下载失败认识是空间不足
						if (CBgDownloadMgr::GetInstance()->CheckFreeSpaceLowerThan(FREE_SPACE_MIN_SIZE))
						{
							nStatus = VERIFY_ERR_INNER_CODE_LOW_FREE_SPACE;
							CPackLogSender::GetInstance()->OnPackDownCallback(STATUS_NOT_ENOUGH_SPACE, pszFileName);
						}
					}
					CPackLogSender::GetInstance()->NetDownLog("debug", "BG DownFail Addr=%d Status=%d FileName=%s Version=%d", nAddr, nStatus, pszFileName, nPackVersion);
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_DOWN_FILE_ERROR, strFileFullPath.c_str(), nStatus, true);
				}
			}
		}
	}


	// 下载项网络的监听 单独监听
	void CBgResDownloadListen::OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo)
	{
		if (enStatus != EnNet_Connect)
		{
			CPackLogSender::GetInstance()->SetPackDownError(ERROR_TYPE_CONNECT, (int)enStatus);
		}
	}

	void CBgResDownloadListen::GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount)
	{
		CBgDownloadMgr::GetInstance()->SetDownloadSpeed(c3pack_down::Double2Int(dDownloadSpeed));
	}
}