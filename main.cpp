#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <tchar.h>

#include "lib/httplib.h"
#include "utils/utils.h"
#include "utils/codingUtils.h"
#include "lib/json.hpp"

using std::cout;
using std::cin;
using std::string;
using std::regex;
using std::regex_search;
using std::smatch;
using json = nlohmann::json;
using namespace httplib;

const string CURRENT_VERSION =  "1.5";

// 显示欢迎页面
void showInfo(){
    cout<<"************************************************\n";
    cout<<"欢迎使用南邮校园网自动登录小程序！\n";
    cout<<"@Author  : 咖啡有点甜\n";
    cout<<"@Data    : 2021.10.07\n";
    cout<<"@Version : 1.5\n";
    cout<<"@Tip     : 有任何问题请添加工作微信:fancy-coffee\n";
    cout<<"************************************************\n";
    
}

// 检测更新
void checkUpdate(){
    
    Client cli("http://mycoffee.top:60002");
    Headers headers = {
        {"key", "NJUPT-WIFI"}
    };
    auto res = cli.Get("/njuptwifi", headers);

    if(res->status!=200){
        cout<<"\n\n检测更新失败了，您可以前往 mycoffee.top 查看最新版本哦！";
        Sleep(1000*10);
        return;
    }

    // string r =  res->body;
    string r = UtfToGbk(res->body.c_str());

    // 解析json
    auto j = json::parse(r);

    // 数据效验正确
    if(j.contains("ret")&&j.at("ret")=="true"){

        auto data = j.at("data");
        bool isForceUpdate = data.at("isForceUpdate");
        string version = data.at("version");
        string detail = data.at("detail");


        // 需要强制更新
        if(isForceUpdate&&version!=CURRENT_VERSION){
            
            // 输出提示
            cout<<"\n\n当前版本太旧了，需要立即更新！";
            cout<<"\n更新说明:\n";
            cout<<UtfToGbk(detail.c_str());

            // 显示按钮
            HWND zmHwnd = FindWindow(_T("Progman"), _T("Program Manager"));
            HWND bzHwnd = FindWindowEx(zmHwnd, 0, _T("SHELLDLL_DefView"), NULL);
            HWND tbHwnd = FindWindowEx(bzHwnd, 0, _T("SysListView32"), _T("FolderView"));
            MessageBox(tbHwnd, _T("当前版本太旧了，请尽快更新哦！"), _T("更新提示"), MB_OK);

            // 确认后等待3s跳转
            Sleep(1000*3);
            string url = j.at("data").at("downloadUrl");
            string cmd = "start "+url;
            system(cmd.c_str());
        } 
        // 无须强制更新
        else if(!isForceUpdate&&version!=CURRENT_VERSION){
            string downloadUrl = data.at("downloadUrl");
            cout<<"\n\n检测到新版本！";
            cout<<"\n更新说明:\n";
            cout<<UtfToGbk(detail.c_str());
            cout<<"\n您可以前往 "<<downloadUrl<<" 下载哦！";
            cout<<"\n如果上述网站不能打开，请前往 mycoffee.top 查看最新版本哦！";
            Sleep(1000*10);
        }
        // 最新版
        else if(version==CURRENT_VERSION){
            cout<<"\n\n您使用的是最新版哦！";
            Sleep(1000*5);
        }
        
    } else {
        cout<<"检测更新失败了，您可以前往 mycoffee.top 查看最新版本哦！";
        Sleep(1000*10);
    }
}


// 初始化
void init(string &account,string &password,string &isp){
    // 获取用户本地路径
    string userPath = getUserPath();
    // 新建文件夹
    newDir(userPath+"\\.coffee");
    // 初始化信息存储位置
    string userFile = userPath + "\\.coffee\\user.txt";

    cout<<"\n如需重新配置账号密码等，请删除 "<<userPath + "\\.coffee"<< "下的user.txt文件(直接复制路径到文件管理器即可)\n";

    // 配置文件不存在
    if (!isFileExiest(userFile)){

        int isp_num=0;
        string needStart="";

        cout<<"\n检测到配置文件不存在，重新初始化！\n";
        cout<<"1.请输入学号:";
        cin>>account;

        cout<<"2.请输入密码:";
        cin>>password;

        cout<<"3.选择您的运营商:\n (1):中国移动CMCC\n (2):中国电信CHINANET\n (3):默认校园网NJUPT\n";
        cout<<"请选择运营商(输入序号即可,无须括号):";
        cin>>isp_num;
        if(isp_num==1){
            isp = "NJUPT-CMCC";
        } else if (isp_num==2){
            isp = "NJUPT-CHINANET";
        } else if (isp_num==3){
            isp = "NJUPT";
        } else{
            cout<<"输入有误，程序即将退出，请重新配置！";
            Sleep(3000);
            exit(0);
        }
        
        cout<<"4.是否需要设置开机启动(y/n):";
        cin>>needStart;

        if(needStart=="y"||needStart=="Y"){
            needStart="1";
        } else{
            needStart="0";
        }

        // 保存
        std::ofstream OutFile(userFile); 
        OutFile << account<<","<<password<<","<<isp<<","<<needStart; 
        OutFile.close(); 

        cout<<"\n配置保存成功！将在5秒内退出程序，重新启动即可正常使用！";
        Sleep(1000*5);
        exit(0);
    } else {
        string needStart;
        cout<<"\n检测到配置文件已存在！";

        std::ifstream readFile(userFile);
        char temp[1024] = { 0 };
        readFile >> temp; 
        // cout << temp;
        readFile.close();

        int length;
        string st = temp;
        std::istringstream stream(st);
        int i = 0;
        char array[100] = { 0 };

        std::vector<string> info;
        while (stream.get(array, 100, ','))
        {
            length = stream.tellg();
            info.push_back(string(array));
            stream.seekg (length + 1, std::ios::beg);
            i++;
            
        }

        // 配置
        account = info[0];
        password = info[1];
        isp = info[2];
        needStart = info[3];

        // 开机启动
        if(needStart=="1"){
            setReStart();
        }
    }

}

void connectWIFI(string wifiName){
    cout<<"\n\n正在连接指定WIFI~\n";

    // 获取连接的信息
    const char cmd[] = {"netsh wlan show interfaces"};
    char result[1024];
    executeCMD(cmd, result);
    string r(result);

    // 正则
    smatch match;
    regex pattern("SSID                   : (.*?)\n"); // 匹配SSID

    // 匹配成功:连上了WIFI
    if (regex_search(r, match, pattern))
    {
        
        string currentSSID = match.str(1);
        // 当前连上的WIFI是目标WIFI时 返回true
        if(currentSSID==wifiName){
            // return true;
            cout<<"\n已连上目标WIFI~\n";
            return;
        }

    }

    // 连接WIFI
    string c_cmd = "netsh wlan connect name=" + wifiName;
    char c_result[1024];
    executeCMD(c_cmd.c_str(), result);
    string c_r(c_result);

    if (c_r.find("已成功完成连接请求") != c_r.npos){
        cout<<"\n连接执行完毕！";
        return;
    }
    // cout<<"\n连接执行失败！";
}

// 连接指定WIFI
void waitConnect(){

    string ip = "jwc.njupt.edu.cn";
    while(1){
        string cmd = "ping -n 1 -w 1000 "+ip+" > NUL";
        int r = system(cmd.c_str());
        
        if(r==0){
            cout<<"\nWIFI连接成功！";
            break;
        }
        Sleep(100);
    }


}

// 获取ip参数
void get_ip(string &ip,string &ac_ip,string &ac_name){
    string url = "http://www.360.com";
    Client cli(url.c_str());
    Headers headers = {
        {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36 Edg/91.0.864.67"}
    };
    auto res = cli.Get("/", headers);
    // 响应体
    string body = UtfToGbk(res->body.c_str());

    smatch match;

    // cout<<res->status;

    if (res->status==200){

        // 宿舍类型网络
        if (body.find("Authentication")!=body.npos){
            regex pattern("ip=(.*?)&wlanacip=(.*?)&wlanacname=(.*?)\"");
            if (regex_search(body, match, pattern)){
                // user.setParam(match.str(1),match.str(2),match.str(3));
                // cout<<match.str(1)<<match.str(2)<<match.str(3);
                ip = match.str(1);
                ac_ip =match.str(2);
                ac_name=match.str(3);
                cout<<"\n\nip获取成功！";
            }
        }
        // 图书馆类型网络
        else if (body.find("NextURL") != body.npos)
        {
            regex pattern("UserIP=(.*?)&wlanacname=(.*?)&(.*?)=");
            if (regex_search(body, match, pattern)){
                // cout<<match.str(1)<<match.str(3)<<match.str(2);
                ip = match.str(1);
                ac_ip =match.str(3);
                ac_name=match.str(2);
                cout<<"\n\nip获取成功！";
            }
        }
        // 成功登陆
        else if (body.find("360手机卫士") != body.npos)
        {
            cout<<"\n\n已经登陆成功咯！";
            checkUpdate();
            exit(0);

        }

    }
}

// 开始连接
void connect(string account,string password,string isp){

    string ip;
    string ac_ip;
    string ac_name;
    
    get_ip(ip,ac_ip,ac_name);

    if (isp == "NJUPT-CMCC")
     account = ",0," + account + "@cmcc";
    else if (isp == "NJUPT-CHINANET")
        account = ",0," + account + "@njxy";
    else if (isp == "NJUPT-CHINANET")
        account = ",0," + account;


    Params params{
            {"R1", "0"},
            {"R2", "0"},
            {"R3", "0"},
            {"R6", "0"},
            {"para", "0"},
            {"0MKKey", "123456 "},
            {"buttonClicked", ""},
            {"redirect_url", ""},
            {"err_flag", ""},
            {"username", ""},
            {"password", ""},
            {"user", ""},
            {"cmd", ""},
            {"Login", ""},
            {"v6ip", ""}
        };
    params.emplace("DDDDD", account);
    params.emplace("upass", password);

    Headers headers = {
        {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"},
        {"Accept-Encoding", "gzip, deflate"},
        {"Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6"},
        {"Cache-Control", "max-age=0"},
        {"Connection", "keep-alive"},
        {"Content-Length", "167"},
        {"Content-Type", "application/x-www-form-urlencoded"},
        {"Host", "p.njupt.edu.cn:801"},
        {"Origin", "http://p.njupt.edu.cn"},
        {"Referer", "http://p.njupt.edu.cn/"},
        {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.75 Safari/537.36 Edg/86.0.622.38"}
    };

    Client cli("http://p.njupt.edu.cn:801");
    string baseUrl = "/eportal/?c=ACSetting&a=Login&protocol=http:&hostname=p.njupt.edu.cn" \
              "&iTermType=1&wlanuserip="+ ip +"&wlanacip="+ ac_ip +"&wlanacname="+ ac_name +"&mac=00-00-00-00-00" \
              "-00&ip="+ ip +"&enAdvert=0&queryACIP=0&loginMethod=1";
    auto res = cli.Post(baseUrl.c_str(),headers,params);
    if(res->status==200){
        cout<<"\n连接请求发起成功！";
    }
    // // 响应体
    // string body = res->body;
}

// 检测连接
void checkConnect(){



    string url = "http://www.360.com";
    Client cli(url.c_str());
    Headers headers = {
        {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36 Edg/91.0.864.67"}
    };
    auto res = cli.Get("/", headers);
    string body = UtfToGbk(res->body.c_str());

    // cout<<UtfToGbk(body.c_str());

    if(res->status==200){
        if (body.find("360") != body.npos)
        {
            cout<<"\n\n已经登陆成功咯~~~";
            checkUpdate();
            // 检测更新
        }
        else{
            cout<<"\n\n登陆失败！请检查账号密码是否有误!\n";
            cout<<"\n如需重新配置账号密码等，请删除 "<<getUserPath() + "\\.coffee"<< "下的user.txt文件(直接复制路径到文件管理器即可)\n";
            Sleep(1000*10);
        }
    } else{
        cout<<"\n\n登陆失败！校园网可能断开！\n";
        Sleep(1000*10);
    }



    

}


int main(){
    // 账号
    string account; 
    // 密码
    string password;
    // 运营商
    string isp;

    // 显示欢迎页
    showInfo();
    //初始化
    init(account,password,isp);

    connectWIFI(isp);
    // 等待连接WIFI
    waitConnect();
    // 开始启动连接
    connect(account,password,isp);
    // 检测连接是否成功
    checkConnect();

    // system("pause");
    return 0;
}