#include <iostream>
#include<stdlib.h>
#include "utils.h"
using namespace::std;


void connectWIFI(string wifiName){
    string cmd = "start /b netsh wlan show interfaces";
    system(cmd.c_str());
}


int main(){

    connectWIFI("123");

    // cout<<getenv("USERNAME");
    system("pause > nul");
    return 0;
}