#pragma once
#include <Windows.h>
#include <iostream>
#include <io.h>
#include <tchar.h>
const std::string DEFAULT_SAVE_PATH = "C:\\coffee";

// 获取存储目录
std::string getUserPath(){
    std::string userEnv = getenv("homepath");
    if (userEnv.length()==0){
        return DEFAULT_SAVE_PATH;
    }

    // default: c:/coffee
    return "c:"+userEnv;    
}

// 获取用户名
std::string getUserName(){
	return std::string(getenv("USERNAME"));
}

// 新建文件夹
void newDir(std::string path){
    // string cmd = "mkdir "+ path;
    // int r = system(cmd.c_str());

    if (access(path.c_str(), 0) !=0 )
    {
        // if this folder not exist, create a new one.
        mkdir(path.c_str());   // 返回 0 表示创建成功，-1 表示失败
        //换成 ::_mkdir  ::_access 也行，不知道什么意思
    }
}

// 设置开机启动
void setReStart() {
	HKEY hKey;
	// 打开注册表
	RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_ALL_ACCESS,&hKey);
	// 获取当前exe路径
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL,szExePath,MAX_PATH);
	// 往注册表写信息
	RegSetValueEx(hKey, _T("NJUPTWIFI"), 0, REG_SZ, (BYTE*)szExePath, MAX_PATH);
	// 关闭注册表
	RegCloseKey(hKey);
}


// 判断文件是否存在
bool isFileExiest(std::string filePath){
    return _access(filePath.c_str(),0) == -1?false:true;
}

// 执行cmd
void executeCMD(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024] = {0};
    FILE *ptr;
    strcpy(ps, cmd);
    if ((ptr = popen(ps, "r")) != NULL)
    {
        while (fgets(buf_ps, 1024, ptr) != NULL)
        {
            strcat(result, buf_ps);
            if (strlen(result) > 1024)
                break;
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
}










