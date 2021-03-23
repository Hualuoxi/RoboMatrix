#ifndef _DATAHANDLING_H
#define _DATAHANDLING_H

#include <unordered_map>
#include "stdlib.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

//服务器数据类型
struct ServersData
{
    /* data */
    string server_type; 
    int cpu;   //cpu数
    int memory; // 内存大小
    int hardware_cost; // 硬件成本
    int energy_day; // 每天成本
};
//虚拟机数据类型
struct VMData
{
    string vm_type; 
    int cpu;   //cpu数
    int memory; // 内存大小
    int node;  //单节点 0  双节点 1
};
//单个请求数据
struct RequestData  
{
    string req;   //add del
    int id;    //添加或者删减的id
    string vm_type; //虚拟机型号  如果是删减  就没有型号
};
//一天的请求数据  
struct DayRequestData
{
    vector<RequestData> day_request;  //所有数据  顺序存储
	vector<RequestData> add_req;     //当天所有增加请求
	vector<RequestData> del_req;    //当天所有删除请求
    int DayPeak_CPU;
	int DayPeak_Mem;
	int DayNow_CPU;
	int DayNow_Mem;
};

class DataHandling
{
private:
    int servers_num,vms_num,day_num;
    vector<int> everyday_num;
    ServersData servers_data;
    VMData vms_data;
    RequestData req_data;
    int ser_tmp,vm_tmp,req_tmp;
    int day_tmp=0;
    bool debug;
    int i=1;
    int num=1; 

    string::iterator  p ; //迭代器p
    void serversDeal(string _word);
    void vmsDeal(string _word);
    void requestsDeal(string _word);   
public:
    int Peak_CPU = 0;	//全局服务器使用峰值
	int Peak_Mem = 0;	//全局内存使用峰值
	int Now_CPU;		//当前使用CPU总数
	int Now_Mem;		//当前使用内存总数
    unordered_map <string, ServersData> servers;  //所有服务器
    unordered_map <string, VMData> vms;   //所有虚拟机
    vector<DayRequestData> *requests_all;  //所有天的请求数据
    DataHandling(bool _debug);
    //是否读取完所有数据
    bool dealLineData(string tmp_line);
    ~DataHandling(void);
    bool openFile(const char *filePath);
    void stdCout( );

};


#endif