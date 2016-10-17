#include<Windows.h>
#include<cstring>
#include<cstdlib>
#include<iostream>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <wchar.h>
using namespace std;

#pragma comment(lib, "User32.lib")

char path[] = "D:\\Filme\\*";

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

char *returnOnlyName(const wchar_t *path) {
	char new_path[100000];
	int i;
	wcstombs(new_path, path,sizeof(new_path));
	
	for (i =0; i< strlen(new_path) - 1; i++) {
		if (new_path[i] == '\\')
			break;
	}

	strcpy(new_path, new_path + i + 1);

	for (i = strlen(new_path) - 1; i>=0 ; i--) {
		if (new_path[i] == '\\')
			break;
	}

	strcpy(new_path + i, new_path + i + 2);

	return new_path;
}

boolean createRegEntryFolder(const wchar_t *path) {
	HKEY hkey;
	wstring new_path = wstring((const wchar_t*)L"SOFTWARE\\CSSO\\") + wstring(convertCharArrayToLPCWSTR(returnOnlyName(path)));	
	const wchar_t *npath = new_path.c_str();

	if (RegCreateKeyEx(HKEY_CURRENT_USER, npath, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL) ==0)
		return true;
	RegCloseKey(hkey);//
	return false;
}

boolean createRegEntryFile(const wchar_t *path,const wchar_t *fileName,const BYTE *filesize) {
	HKEY hkey;
	wstring folder_path = wstring((const wchar_t*)L"SOFTWARE\\CSSO\\") + wstring(convertCharArrayToLPCWSTR(returnOnlyName(path)));
	folder_path.pop_back();
	folder_path.pop_back();
	const wchar_t *npath = folder_path.c_str();

	LONG openHandle = RegOpenKeyEx(HKEY_CURRENT_USER, npath, 0, KEY_ALL_ACCESS, &hkey);
	if (openHandle == 0) {
		if (RegSetValueEx(hkey, fileName, 0, REG_DWORD, filesize, sizeof(filesize)) == 0)
			return true;
		return false;
	}
	RegCloseKey(hkey);//
	return false;
}


boolean parseFolder(const wchar_t *path) {
	WIN32_FIND_DATA ffd;
	HANDLE handle = FindFirstFile(path, &ffd);//"."
	FindNextFile(handle, &ffd);//".."

	if (handle != INVALID_HANDLE_VALUE) {
		if (FindNextFile(handle, &ffd) == 0) {
			return true;
		}
		else {
			do
			{
				wstring newpath = wstring(path).substr(0, wstring(path).length() - 1);
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					wstring newpath = wstring(path).substr(0, wstring(path).length() - 1) + wstring(ffd.cFileName) + L"\\*";
					//daca esueaza un folder ne oprim
					if (createRegEntryFolder(newpath.c_str()) == false) {
						cout << "Path-ul " << _TEXT("%s",newpath.c_str())<< " a esuat" << endl;
						wcout << newpath.c_str();
						goto _exit;
					}

					//daca esueaza un path ne oprim
					if (parseFolder(newpath.c_str()) == false) {
						cout << "Path-ul " << _TEXT("%s", newpath.c_str()) << " a esuat" << endl;
						wcout << newpath.c_str();
						goto _exit;
					}

				}
				else
				{
					//const BYTE* fileBYTE = new BYTE();

					//daca esueaza un file ne oprim
					if (createRegEntryFile(newpath.c_str(), wstring(ffd.cFileName).c_str(), (const BYTE*)&ffd.nFileSizeLow) == false) {
						cout << "Path-ul " << _TEXT("%s", newpath.c_str()) << " a esuat" << endl;
						wcout << newpath.c_str();
						goto _exit;
					}

				}
			} while (FindNextFile(handle, &ffd) != 0);
			return true;
		}
	}
	_exit:
	CloseHandle(handle);
	return false;
}

int main() {

	if (parseFolder(convertCharArrayToLPCWSTR(path)) == 1)
			cout << "Operatie reusita" << endl;
	else
			cout << "Operatia a esuat" << endl;

	system("pause");
	return 0;
}