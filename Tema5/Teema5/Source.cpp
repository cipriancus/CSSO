#include<Windows.h>
#include<cstring>
#include<cstdlib>
#include<iostream>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <wchar.h>
#include <aclapi.h>
using namespace std;

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

boolean createRegEntryFolder(const char *name) {
	HKEY hkey;
	wstring new_path = wstring((const wchar_t*)L"SOFTWARE\\") + wstring(convertCharArrayToLPCWSTR(name));

	SECURITY_ATTRIBUTES sa;
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
	
	if (pSD == NULL) {
		return false;
	}

	if (!InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION)){
		return false;
	}

	PACL pACL = NULL;
	DWORD dwRes;
	EXPLICIT_ACCESS ea[2];
	PSID pAdminSID,pRestulSID;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

	if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pRestulSID))
	{
		return false;
	}

	if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdminSID))
	{
		return false;
	}

	//Read pe restul
	ea[0].grfAccessPermissions = KEY_READ;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pRestulSID;

	//Read, write pe contul curent
	ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
	ea[1].grfAccessMode = GRANT_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;

	dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		return false;
	}
	
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		return false;
	}
	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, new_path.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, &sa, &hkey, NULL) == 0) {
		RegCloseKey(hkey);
		return true;
	}
	RegCloseKey(hkey);
	return false;
}

int main() {
	
	const char* numePath = "CSSO";
	const char* numeFile = "tema5file";

	if (createRegEntryFolder(numePath)== false) {
		cout << "Crearea folder ului a esuat" << endl;
	}

	system("pause");
	return 0;
}