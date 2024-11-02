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
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (std::wstring(pe.szExeFile) == processName) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
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

// 设置鼠标加速开关的函数
void SetMouseAcceleration(BOOL mouseAccel) {
    int mouseParams[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);
    mouseParams[2] = mouseAccel;
    SystemParametersInfo(SPI_SETMOUSE, 0, mouseParams, SPIF_SENDCHANGE);
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
