#include<Windows.h>
#include<cstring>
#include<cstdlib>
#include<iostream>
using namespace std;

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

char *returnOnlyName(wchar_t *path) {
	char new_path[100];
	int i;
	wcstombs(new_path, path,sizeof(new_path));
	

	for (i = strlen(new_path) - 1; i >= 0; i--) {
		if (new_path[i] == '\\')
			break;
	}
	return strcpy(new_path, new_path + i + 1);;
}

void createRegEntry(wchar_t *path) {
	HKEY hkey;
	char new_path[100]= "SOFTWARE\\CSSO\\";

	strcat(new_path,returnOnlyName(path));
	
	RegCreateKeyEx(HKEY_CURRENT_USER, convertCharArrayToLPCWSTR(new_path), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);
}

boolean parseFolder(HANDLE handle) {
	for () {
		//parsam toate file-urile
		if()//daca e fisier parseFolder(path)==false;break si eroare
		else
			//daca e file createRegEntry(path);
	}
	return true;
}

int main() {
	char path[] = "D:\\Muzicile";

	HANDLE handle = CreateFile(convertCharArrayToLPCWSTR(path), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	
	if (handle == INVALID_HANDLE_VALUE) {
		cout << "Path invalid"<<endl;
		
		if (GetLastError() == ERROR_FILE_NOT_FOUND){
			//boolean flag = CreateDirectory(convertCharArrayToLPCWSTR(path), NULL);
			//createRegEntry(convertCharArrayToLPCWSTR(path));
		}

	}
	else {//parcurgere recursiva
		if (parseFolder(handle) == 1)
			cout << "Operatie reusita" << endl;
		else
			cout << "Operatia a esuat" << endl;
	}

	cout << GetLastError();
	CloseHandle(handle);
	system("pause");
	return 0;
}