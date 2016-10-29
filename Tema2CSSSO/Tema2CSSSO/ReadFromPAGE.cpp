#include<Windows.h>
#include<stdio.h>
#include<TlHelp32.h>
#include<iostream>
#include <tchar.h>
#include <wchar.h>
#include <string.h>
#include<map>
#include<fstream>
using namespace std;


int sizeOfPage;

wstring searchPID(WCHAR *token) {
	wstring foundPID = wstring(token);

	int iterator;
	for (iterator = 1; iterator < foundPID.size(); iterator++) {
		if (foundPID[iterator] == '>')
			break;
	}
	return foundPID.substr(1, iterator - 1);
}

wstring searchPPID(WCHAR *token) {
	wstring foundPPID = wstring(token);

	int iterator;
	for (iterator = 1; iterator < foundPPID.size(); iterator++) {
		if (foundPPID[iterator] == '>')
			break;
	}

	foundPPID = foundPPID.substr(iterator + 1, foundPPID.size() - 1);

	for (iterator = 1; iterator < foundPPID.size(); iterator++) {
		if (foundPPID[iterator] == '>')
			break;
	}



	return foundPPID.substr(1, iterator - 1);
}

wstring search(wstring PPID, TCHAR* PAGE, int tab) {

	TCHAR* copyPAGE = (TCHAR*)malloc(sizeOfPage);
	CopyMemory(copyPAGE, PAGE, sizeOfPage);

	wstring output = L"";



	WCHAR *buffer;
	WCHAR *token = wcstok(PAGE, L"\n", &buffer);
	while (token != NULL) {
		if (searchPPID(token) == PPID && searchPPID(token) != searchPID(token)) {//are procesul parinte egal si nu e parinte sau
			for (int iterator = 0; iterator < tab; iterator++)
				output = output + L" ";

			output = output + token + L"\n";//adaug in arbore

			TCHAR* PAGEcopy = (TCHAR*)malloc(sizeOfPage);
			CopyMemory(PAGEcopy, copyPAGE, sizeOfPage);//copiem pentru a face strtok

			output = output + search(searchPID(token), PAGEcopy, tab + 1);//trimit ce pid parinte sa caute
			output = output + L"\n";
		}
		token = wcstok(NULL, L"\n", &buffer);
	}
	return output;
}


//inchide handle-uri
boolean createTREE() {
	HANDLE createNewFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, L"PaginaNouaTema2");

	if (createNewFile == NULL)
	{
		printf("Could not create file mapping object (%d).\n", GetLastError());
		CloseHandle(createNewFile);
		cout << GetLastError();
		return false;
	}

	LPVOID  newFILEbuffer = MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	TCHAR* p = (TCHAR*)newFILEbuffer;

	if (newFILEbuffer == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		CloseHandle(createNewFile);
		UnmapViewOfFile(newFILEbuffer);
		return false;
	}


	sizeOfPage = _tcslen(p) * sizeof(TCHAR*);

	TCHAR* newPAGEcopy = (TCHAR*)malloc(sizeOfPage);//copiez pagina ca sa nu  o strice strtok
	memcpy(newPAGEcopy, p, (sizeOfPage));

	wstring output;
	WCHAR *buffer;
	int no = 0;
	WCHAR *token = wcstok(p, L"\n", &buffer);
	while (token != NULL) {
		if (true) {//e proces parinte
			output.clear();
			WCHAR buffer_no[10];
			_itow(++no, buffer_no, 10);
			output = output + L"[arborele "+ wstring(buffer_no)+ L"]\n" + token + L"\n";//numesc arborele

			TCHAR* PAGEcopy = (TCHAR*)malloc(sizeOfPage);
			CopyMemory(PAGEcopy, newPAGEcopy, sizeOfPage);//copiem pentru a face strtok

			output = output + search(searchPID(token), PAGEcopy, 1);//trimit ce pid parinte sa caute
			wcout << output.c_str() << endl;
		}
		token = wcstok(NULL, L"\n", &buffer);
	}
	return true;
}

int main() {

	if (createTREE() == false) {
		printf("Opperation failed %d", GetLastError());
		return(-1);
	}
	
	cout << "Opperation successful";

	return 0;
}

/*
HANDLE processToken;

if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processToken) == false) {
CloseHandle(processToken);
return false;
}

LUID privilegeID;

if (LookupPrivilegeValueW(NULL,SE_DEBUG_NAME, &privilegeID)==false) {
return false;
}

TOKEN_PRIVILEGES newPrivileges;
newPrivileges.PrivilegeCount = 1;
newPrivileges.Privileges[0].Luid = privilegeID;

if (AdjustTokenPrivileges(processToken,false,&newPrivileges,sizeof(TOKEN_PRIVILEGES),NULL,0)==false) {
cout << GetLastError();
return false;
}


return true;*/