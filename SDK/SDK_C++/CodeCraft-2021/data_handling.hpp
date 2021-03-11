#ifndef DATAHANDLING
#define DATAHANDLING

#include <unordered_map>
#include "stdlib.h"
#include <vector>
#include <fstream>
#include<sstream>
#include<iostream>
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
    int single;  //是否为单节点  0  1
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
    vector<RequestData> day_request;
};


class DataHandling
{
public:
    unordered_map <string, ServersData> servers;  //所有服务器
    unordered_map <string, VMData> vms;   //所有虚拟机
    vector<DayRequestData> *requests_all;  //所有的请求数据
    DataHandling(bool _debug=true)
    {
        debug = _debug;
    };
    //~DataHandling();
    bool openFile(char *filePath)
    {
        ifstream infile(filePath,std::ios::in);
        string tmp_line;
        string::iterator  p ; //迭代器p
        cout <<"path: " <<filePath << "\n";
        int i=1;
        int num=1; day_tmp = 0;
        if (!infile.fail())
        {          
            while(getline(infile, tmp_line ) && !infile.eof())  //逐行读取
            {
                if(debug) cout << i <<"\n"; i=i+1;
                if(tmp_line.data()[0] != '(' ) //
                {
                    
                    if(num == 1) servers_num = stoi(tmp_line);
                    if(num == 2) vms_num = stoi(tmp_line);
                    if(num == 3) {
                        day_num = stoi(tmp_line);
                        requests_all = new vector<DayRequestData>(day_num);
                    }
                    if(num > 3 ) 
                    {
                        everyday_num.push_back( stoi(tmp_line) );
                        day_tmp++;
                    }
                    num+=1;
                    if(debug) cout<<"num:  "<<stoi(tmp_line) <<"\n";
                }
                else
                {
                    p = tmp_line.begin();
                    tmp_line.erase(p);  //cout << tmp_line << endl;
                    p = tmp_line.end(); p--;
                    tmp_line.erase(p); //cout << tmp_line << endl;
                    stringstream words(tmp_line);
                    string tep_word;
                    if(debug) cout <<"line:  " <<tmp_line <<endl;
                    ser_tmp = 5; vm_tmp = 4;req_tmp=3;
                    while(getline(words, tep_word, ','))//以,为分隔符，读取数据
                    {
                       // cout << tep_word << "\t";
                        if(num == 2) serversDeal(tep_word);
                        if(num == 3) vmsDeal(tep_word);
                        if(num>3)  requestsDeal(tep_word);
                    }

                    if(num ==2) 
                    {
                        pair<string,ServersData>tmp (servers_data.server_type,servers_data);
                        servers.insert(tmp );
                    }
                    if(num == 3)
                    {
                        pair<string,VMData>tmp (vms_data.vm_type,vms_data);
                        vms.insert(tmp);
                    }
                    
                    if(day_tmp>0 )
                    {
                        requests_all->at(day_tmp-1).day_request.push_back(req_data);
                    }

                    if(debug) cout<<"\n";
                }
            }
            cout<<"read over\n";
        }
        else
        {
            return false;
        }
        
        infile.close();
        cout<<"close file\n";
        return true;
    }
private:
    int servers_num,vms_num,day_num;
    vector<int> everyday_num;
    ServersData servers_data;
    VMData vms_data;
    RequestData req_data;
    int ser_tmp,vm_tmp,req_tmp;
    int day_tmp;
    bool debug;
    void serversDeal(string _word)
    {
         if(ser_tmp == 5){
                servers_data.server_type = _word;
                ser_tmp--;
                if(debug) cout << servers_data.server_type<<"  ";
            }
            else if(ser_tmp == 4){
                servers_data.cpu = stoi(_word);
                ser_tmp--;
                if(debug) cout << servers_data.cpu<<"  ";
            }
            else if(ser_tmp == 3){
                servers_data.memory = stoi(_word);
                ser_tmp--;
                if(debug) cout << servers_data.memory<<"  ";
            }
            else if(ser_tmp == 2){
                servers_data.hardware_cost = stoi(_word);
                ser_tmp--;
                if(debug) cout << servers_data.hardware_cost<<"  ";
            }
            else if(ser_tmp == 1){
                servers_data.energy_day = stoi(_word);
                ser_tmp--;
                if(debug) cout << servers_data.energy_day<<"  ";
            }
    }

    void vmsDeal(string _word)
    {
        if(vm_tmp == 4){
            vms_data.vm_type = _word;
            ser_tmp--;
            if(debug) cout << vms_data.vm_type<<"  ";
        }
        else if(vm_tmp == 3){
            vms_data.cpu = stoi(_word);
            ser_tmp--;
            if(debug) cout << vms_data.cpu<<"  ";
        }
        else if(vm_tmp == 2){
            vms_data.memory = stoi(_word);
            ser_tmp--;
            if(debug) cout << vms_data.memory<<"  ";
        }
         else if(vm_tmp == 1){
            vms_data.single = stoi(_word);
            ser_tmp--;
            if(debug) cout << vms_data.single<<"  ";
        }
    }

    void requestsDeal(string _word)
    {
        if(req_tmp == 3){
            req_data.req = _word;
            req_tmp--;
            if(debug) cout<<"req: " << req_data.req <<"...";
        }
        else if(req_tmp == 2)
        {
            if(req_data.req == "add")
            {
                req_data.vm_type = _word;
                req_tmp--;
               if(debug)  cout<<"type: " << req_data.vm_type<<"...";
            }
            else
            {
                req_data.id = stoi(_word);
               if(debug)  cout<<"id: " << req_data.id <<"...";
            } 
        }
        else if(req_tmp == 1)
        {
            req_data.id = stoi(_word);
            if(debug) cout <<"id: "<< req_data.id <<"...";
        }
    }
};


#endif