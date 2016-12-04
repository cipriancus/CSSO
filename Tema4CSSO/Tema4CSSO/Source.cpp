#include<Windows.h>
#include<stdio.h>
#include<TlHelp32.h>
#include<iostream>
#include <tchar.h>
#include <wchar.h>
#include <string>
#include<map>
#include<fstream>
#include <cstdlib>
#include <ctime>
#include<vector>
#include<regex>
#include<list>
#include<WinInet.h>
using namespace std;

#pragma comment(lib,"Wininet")
#define SERVER_ADDR L"ftp://localhost"

string path = "C:\\Users\\Ciprian\\Downloads\\";

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

bool verifyURL(char line[]) {
	regex e("((http://)|(https://)|(www.))(.*)(.exe)");
	return regex_match(line, e);
}

string take_out_extension(string url) {
	int find_http = url.find("http://") == -1 ? url.find("https://") : url.find("http://");
	int find_www = url.find("www.");

	if (find_http != -1) {
		int is_https = url.find("https://") == -1 ? 0 : 1;
		url = url.substr(find_http + 7 + is_https, url.size());
	}

	if (find_www != -1) {
		url = url.substr(find_http + 5, url.size());
	}
	return url;
}

wchar_t *get_base_domain(string url) {
	url = take_out_extension(url);
	int iterator;
	for (iterator = 0; iterator < url.size(); iterator++)
		if (url[iterator] == '\/')
			return convertCharArrayToLPCWSTR(url.substr(0, iterator).c_str());
}

wchar_t *get_domain_address(string url) {
	url = take_out_extension(url);
	return convertCharArrayToLPCWSTR(url.substr(wcslen(get_base_domain(url)), url.size()).c_str());
}

vector<string> findURLinFILE(WIN32_FIND_DATA ffd, HANDLE handle) {
	vector<string>allURL;

	wstring string = L"/" + wstring(ffd.cFileName);

	HANDLE ftpOpenedFILE = FtpOpenFile(handle, string.c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_ASCII, 0);

	if (ftpOpenedFILE ==NULL) {
		cout << "Error";
		InternetCloseHandle(ftpOpenedFILE);
	}
	else {
		CHAR buffer[1024];//o linie poate avea max 1024
		DWORD readbyes;
		list<char> not_complete_line;

		do {
			bool response = InternetReadFile(ftpOpenedFILE, &buffer, sizeof(buffer), &readbyes);

			if (response == true) {//am citit bine				

				int count_lines = 0;

				for (int iterator = 0; iterator < 1024; iterator++) {//il folosim ca buffer
					not_complete_line.push_back(buffer[iterator]);
					if (buffer[iterator] == '\n')
						count_lines++;
				}

				not_complete_line.erase(remove(not_complete_line.begin(), not_complete_line.end(), '\r'), not_complete_line.end());

				while (count_lines > 0) {
					list<char>::iterator it;
					char temporary_line[1024]; 
					int temp_line = 0;
					for (it = not_complete_line.begin(); it != not_complete_line.end(); it++) {
						if ((*it) != '\n') {
							temporary_line[temp_line++] = (*it);
						}
						else {//am gasit o linie, facem pop de toata linia
							while (temp_line != 0) {
								not_complete_line.pop_front();
								temp_line--;
							}
							not_complete_line.pop_front();//elimin si new line

							if ( verifyURL(temporary_line) == true && (find(allURL.begin(),allURL.end(),temporary_line)==allURL.end() || allURL.size()==0))
								allURL.push_back(temporary_line);
							break;
						}
					}
					count_lines--;
					fill_n(temporary_line, 1024, NULL);
				}
			}
		} while (readbyes != 0);
	}
	InternetCloseHandle(ftpOpenedFILE);
	return allURL;
}

//create http request
bool downloadANDexecute(vector<string> urls) {
	vector<string>::iterator it;
	
	HANDLE internetHandle = InternetOpen(L"FTP PROGRAM ACCESSER", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (internetHandle == NULL) {
		cout << "Error";
		return 0;
	}

	for (it = urls.begin(); it != urls.end(); it++) {
		string url = (*it);

		wstring base_domain = wstring(get_base_domain(url));
		HANDLE internetConnect = InternetConnect(internetHandle, get_base_domain(url),INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0,0);

		if (internetConnect == NULL) {
			InternetCloseHandle(internetHandle);
			return false;
		}

		const wchar_t *lplpszAcceptTypes[] = { L"*/*", NULL };
	    wchar_t *p = get_domain_address(url);
		HANDLE httpOpenRequest = HttpOpenRequest(internetConnect, L"GET", get_domain_address(url), L"HTTP/1.1",NULL, 0, NULL, 0);

		if (httpOpenRequest == NULL) {
			cout << GetLastError();
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetHandle);
			return false;
		}

		bool httpSendRequest = HttpSendRequest(httpOpenRequest, NULL, NULL, NULL, 0); 

		if (httpSendRequest !=false) {
			DWORD readbytes;
			char randNo[20]; 
			srand(time(NULL));
			int randomnumber = rand();
			_itoa(randomnumber, randNo, 10);
			string nameOfFile = path + "downloadedFile" + randNo + ".exe";
			ofstream outfile(nameOfFile,ios::binary);
			do {
				BYTE vect[2048] = { 0 };
				bool a = InternetReadFile(httpOpenRequest, vect, 2048, &readbytes);
				for (int iterator = 0; iterator < readbytes; iterator++)
						outfile.put(vect[iterator]);
			} while (readbytes != 0);
			outfile.flush();
			outfile.close();
			system(nameOfFile.c_str());
		}else {
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetHandle);
			cout << GetLastError();
			return false;
		}
	}
	return true;
}

int main() {
	HANDLE internetHandle = InternetOpen(L"FTP PROGRAM ACCESSER", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	
	if (internetHandle == NULL) {
		cout << "Error at InternetOpen";
		return (-1);
	}

	WCHAR domain[] = L"localhost";

	HANDLE openedFTP = InternetConnect(internetHandle, domain, 0, L"cipriancus", L"1234", INTERNET_SERVICE_FTP, 0, 0);

	if (openedFTP == NULL) {
		cout << "Error at contacting FTP server";
		cout << GetLastError();
		InternetCloseHandle(openedFTP);
		InternetCloseHandle(internetHandle);
		return (-1);
	}
	
	WIN32_FIND_DATA ffd;

	HANDLE ftpFILES = FtpFindFirstFile(openedFTP, L"*", &ffd, INTERNET_FLAG_RELOAD, 0);

	if (ftpFILES != INVALID_HANDLE_VALUE) {
			do {
				wcout << ffd.cFileName << endl;
				if (wcsstr(ffd.cFileName,L".txt")!=NULL) {
					vector<string> allURLsFound = findURLinFILE(ffd, openedFTP);

					if (allURLsFound.size() > 0) {
						if (downloadANDexecute(allURLsFound) == false) {
							cout << endl <<"Some downloads failed" << endl;
						}
					}
				}
			} while (InternetFindNextFile(ftpFILES, &ffd) != 0);
	}
	InternetCloseHandle(ftpFILES);
	InternetCloseHandle(openedFTP);
	InternetCloseHandle(internetHandle);
	system("pause");
	return 0;
}