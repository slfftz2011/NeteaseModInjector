#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include <tchar.h>
#include <shellapi.h>

using namespace std;

// 从注册表获取网易MC启动器下载路径
string getPanfuPath() {
    HKEY hKey;
    const char* subKey = "Software\\Netease\\MCLauncher";
    const char* valueName = "DownloadPath";
    char buffer[1024] = {0};
    DWORD bufferSize = sizeof(buffer);
    DWORD type = REG_SZ;

    // 打开注册表项
    if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "";
    }

    // 读取注册表值
    if (RegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)buffer, &bufferSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "";
    }

    RegCloseKey(hKey);
    return string(buffer);
}

// 递归复制目录（包含子目录和文件）
bool copyDirectory(const string& source, const string& dest) {
    // 创建目标目录
    if (!CreateDirectoryA(dest.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((source + "\\*").c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        string fileName = findData.cFileName;
        if (fileName == "." || fileName == "..") {
            continue;
        }

        string srcPath = source + "\\" + fileName;
        string destPath = dest + "\\" + fileName;

        // 如果是目录则递归复制
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!copyDirectory(srcPath, destPath)) {
                FindClose(hFind);
                return false;
            }
        }
        // 如果是文件则直接复制
        else {
            if (!CopyFileA(srcPath.c_str(), destPath.c_str(), FALSE)) {
                FindClose(hFind);
                return false;
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

// 等待日志文件被删除（最多900秒）
bool waitForLogDeletion(const string& logPath) {
    for (int i = 0; i < 900; ++i) {
        // 检查文件是否存在
        if (GetFileAttributesA(logPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            return true; // 文件已删除
        }
        Sleep(1000); // 等待1秒
    }
    return false; // 超时
}

int main() {
    // 设置控制台编码为UTF-8以支持中文显示
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleA("NeteaseModInjector v1.0.0");

    // 获取启动器路径
    string panfu = getPanfuPath();
    if (panfu.empty()) {
        cout << "找不到网易我的世界游戏的安装位置。" << endl;
        system("pause > nul");
        return 0;
    }

    // 主循环（对应批处理的:CL标签）
    while (true) {
        system("cls");
        cout << endl;
        cout << "=====模组安装=====" << endl << endl;
        cout << "请将模组文件拖拽到此处，然后按Enter键（会自动安装到游戏）" << endl << endl;
        cout << "模组文件路径不要包含中文、特殊符号和空格！路径格式示例：D:\\mods" << endl << endl;
        cout << "-----------------------" << endl;
        cout << "请将模组文件拖拽到此处：" << endl << endl;

        // 获取模组路径（mz）
        string mz;
        getline(cin, mz);

        cout << "-----------------------" << endl << endl;
        cout << "请将配置文件拖拽到此处（没有请直接按Enter跳过）：" << endl << endl;

        // 获取配置路径（pz）
        string pz;
        getline(cin, pz);

        // 目标路径
        string modsDest = panfu + "\\Game\\.minecraft\\mods";
        string configDest = panfu + "\\Game\\.minecraft\\config";
        string logPath = modsDest + "\\JuwLBFt.log";

        // 创建日志文件
        ofstream logFile(logPath);
        if (logFile.is_open()) {
            logFile << "3401765#JuwLBFt";
            logFile.close();
        } else {
            cout << "创建日志文件失败！" << endl;
            system("pause > nul");
            continue;
        }

        cout << endl << "等待中...请启动游戏" << endl << endl;

        // 等待日志文件被删除
        if (!waitForLogDeletion(logPath)) {
            cout << "超时未检测到游戏启动" << endl;
            system("pause > nul");
            continue;
        }

        // 执行复制操作
        if (!pz.empty()) { // 对应:ZX标签（有配置文件）
            cout << "正在安装模组..." << endl;
            if (copyDirectory(mz, modsDest)) {
                cout << "------" << endl;
                cout << "正在安装配置文件..." << endl;
                copyDirectory(pz, configDest);
            }
        } else { // 对应:TG标签（无配置文件）
            cout << "正在安装模组..." << endl;
            copyDirectory(mz, modsDest);
        }

        cout << endl << "操作完成" << endl;
        system("pause > nul");
    }

    return 0;
}