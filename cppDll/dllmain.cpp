#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include "pch.h"
#include <Psapi.h>
// DLL 导出
extern "C" __declspec(dllexport) void StartMonitoring(const std::vector<std::wstring>& processNames);
extern "C" __declspec(dllexport) void StopMonitoring();
extern "C" __declspec(dllexport) void SetMouseAcceleration(BOOL mouseAccel);
extern "C" __declspec(dllexport) void TerminateProcessByName(const std::wstring& processName);

// 监控线程的句柄
std::thread monitorThread;
std::atomic<bool> monitoring(false);

// 终止指定名称的进程的函数
void TerminateProcessByName(const std::wstring& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::wcerr << L"创建快照失败" << std::endl;
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (std::wstring(pe.szExeFile) == processName) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    if (TerminateProcess(hProcess, 0)) {
                        std::wcout << L"已终止进程: " << processName << std::endl;
                    }
                    else {
                        std::wcerr << L"终止进程失败: " << processName << std::endl;
                    }
                    CloseHandle(hProcess);
                }
                else {
                    std::wcerr << L"打开进程失败: " << processName << std::endl;
                }
            }
        } while (Process32Next(hSnap, &pe));
    }
    else {
        std::wcerr << L"检索第一个进程失败" << std::endl;
    }

    CloseHandle(hSnap);
}

// 更改壁纸的函数
void SetWallpaper(LPCWSTR wallpaperPath) {
    if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)wallpaperPath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        wprintf(L"壁纸已更改为: %s\n", wallpaperPath);
    }
    else {
        wprintf(L"更改壁纸失败: %s\n", wallpaperPath);
    }
}

// 持续监控并终止特定进程的函数
void MonitorAndTerminateProcesses(const std::vector<std::wstring>& processNames) {
    while (monitoring.load()) {
        for (const auto& processName : processNames) {
            TerminateProcessByName(processName);
        }
        Sleep(1000); // 每秒轮询一次
    }
}

void RestartExplorer() {
    // 终止 Explorer 进程
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "无法创建进程快照" << std::endl;
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (std::wstring(pe.szExeFile) == L"explorer.exe") {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    std::cout << "已终止 Explorer 进程" << std::endl;
                }
            }
        } while (Process32Next(hSnap, &pe));
    }
    else {
        std::cerr << "无法检索第一个进程" << std::endl;
    }

    CloseHandle(hSnap);

    // 重新启动 Explorer
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(L"C:\\Windows\\explorer.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "已重启 Explorer" << std::endl;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "重启 Explorer 失败" << std::endl;
    }
}

// 开始监控进程
void StartMonitoring(const std::vector<std::wstring>& processNames) {
    if (!monitoring.load()) {
        monitoring.store(true);
        monitorThread = std::thread(MonitorAndTerminateProcesses, processNames);
        monitorThread.detach();
    }
}

// 停止监控进程
void StopMonitoring() {
    monitoring.store(false);
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}
void SetRegistryValue(HKEY root, LPCWSTR subKey, LPCWSTR valueName, DWORD data) {
    HKEY hKey;
    if (RegOpenKeyEx(root, subKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, valueName, 0, REG_DWORD, (const BYTE*)&data, sizeof(data));
        RegCloseKey(hKey);
    }
    else {
        wprintf(L"无法打开注册表项: %s\n", subKey);
    }
}


// 设置鼠标加速开关的函数
void SetMouseAcceleration(BOOL mouseAccel) {
    int mouseParams[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);
    mouseParams[2] = mouseAccel;
    SystemParametersInfo(SPI_SETMOUSE, 0, mouseParams, SPIF_SENDCHANGE);
    SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", 0);
    SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarGlomLevel", 0);
    const wchar_t* wallpaperPath = L"C:\\Windows\\Web\\Wallpaper\\Theme1\\img4.jpg";
    RestartExplorer();
    std::vector<std::wstring> processesToTerminate = {
    L"lwclient64.exe",
    L"lwfunctionbar64.exe",
    L"lwhardware64.exe",
    L"lwBarClientApp32.exe"
    //L"PotPlayerMini64.exe"
    };
    for (const auto& processName : processesToTerminate) {
        TerminateProcessByName(processName);
    }

}

// 终止指定名称的进程（新方法）
void TerminateThreadByName(const std::wstring& threadName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    DWORD processes[1024], cbNeeded;
    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        CloseHandle(hSnap);
        return;
    }

    for (unsigned int i = 0; i < (cbNeeded / sizeof(DWORD)); i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) {
                WCHAR processName[MAX_PATH];
                // 获取进程名称
                if (GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(WCHAR))) {
                    if (std::wstring(processName).find(threadName) != std::wstring::npos) {
                        // 在这里可以设置标志，通知线程优雅退出
                        TerminateProcess(hProcess, 0); // 或者调用其他终止机制
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    CloseHandle(hSnap);
}

// DLL 的入口点（可选）
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
