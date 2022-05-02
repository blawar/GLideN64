#include <algorithm>
#include <string>
#include "../PluginAPI.h"
#include "../RSP.h"

int PluginAPI::InitiateGFX(const GFX_INFO & _gfxInfo)
{
	_initiateGFX(_gfxInfo);

	return TRUE;
}

void PluginAPI::FindPluginPath(wchar_t * _strPath)
{
	wcscpy(_strPath, L".");
	return;
}

void PluginAPI::GetUserDataPath(wchar_t * _strPath)
{
	FindPluginPath(_strPath);
}

void PluginAPI::GetUserCachePath(wchar_t * _strPath)
{
	FindPluginPath(_strPath);
}
