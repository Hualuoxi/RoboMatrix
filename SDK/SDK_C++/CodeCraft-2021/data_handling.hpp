#ifndef DATAHANDLING
#define DATAHANDLING

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
};


class DataHandling
{
private:
    vector<int> everyday_num;
    ServersData servers_data;
    VMData vms_data;
    RequestData req_data;
    int ser_tmp,vm_tmp,req_tmp;
    
    bool debug;
    int i=1;
    int num=1; 
    string::iterator  p ; //迭代器p
public:
    unordered_map <string, ServersData> servers;  //所有服务器
    unordered_map <string, VMData> vms;   //所有虚拟机
    vector<DayRequestData> *requests_all;  //所有天的请求数据
	int servers_num=0, vms_num=0, day_num=0, k_num=0; //服务器型号数量  虚拟机型号数量 总天数 提前知晓的天数
	bool readed_kday_reqs  = false;  //是否读完k天的请求
	bool day_read_finished = false;     //是否读取完当天数据
	bool read_all_finished = false;    //是否读取完所有天数据
    int day_tmp=0;
    int deal_day_num = 0;
    DataHandling(bool _debug=true)
    {
        debug = _debug;
    };
    //是否读取完所有数据

    bool dealLineData(string tmp_line)
    {
        stringstream lineStream(tmp_line);
		day_read_finished = false;
        if(tmp_line.data()[0] != '(' ) //
        {
            if(num == 1) 
                lineStream >> servers_num ;
            else if(num == 2) 
                lineStream >> vms_num;
            else if(num == 3)
            {
                lineStream >> day_num;
                lineStream >> k_num;
                requests_all = new vector<DayRequestData>(day_num);
            }
            else if(num > 3 ) 
            {
                everyday_num.push_back(stoi(tmp_line));
                ++day_tmp;

            }
            ++num;
            return false;
        }
        else
        {
            lineStream.ignore(); //去左括号
            if(2 == num)
            {
                getline(lineStream,servers_data.server_type,',');
                lineStream >> servers_data.cpu;
                lineStream.ignore(2);
                lineStream >> servers_data.memory;
                lineStream.ignore(2);
                lineStream >> servers_data.hardware_cost;
                lineStream.ignore(2);
                lineStream >> servers_data.energy_day;
                servers[servers_data.server_type] = servers_data;
            }
            else if(3 == num)
            {
                getline(lineStream,vms_data.vm_type,',');
                lineStream >> vms_data.cpu;
                lineStream.ignore(2);
                lineStream >> vms_data.memory;
                lineStream.ignore(2);
                lineStream >> vms_data.node;
                vms[vms_data.vm_type] = vms_data;
                //cout << vms[vms_data.vm_type].vm_type << endl;
            }
            else if(3 < num)
            {
                getline(lineStream,req_data.req,',');
                if("add" == req_data.req)
                {
                    lineStream.ignore();
                    getline(lineStream,req_data.vm_type,',');
                }
                lineStream >> req_data.id;
                // cout << req_data.vm_type<<endl;
            }
			if (day_tmp > 0)
			{
				if (everyday_num.at(day_tmp - 1) != 0)  //当天请求数据不为0
				{
					requests_all->at(day_tmp - 1).day_request.push_back(req_data);
					if (req_data.req == "add")
						requests_all->at(day_tmp - 1).add_req.push_back(req_data);
					else
						requests_all->at(day_tmp - 1).del_req.push_back(req_data);
				}
				if (requests_all->at(day_tmp - 1).day_request.size() == everyday_num.at(day_tmp - 1))
                {
					day_read_finished = true;
                    if (day_tmp >= k_num)  
				 	    readed_kday_reqs = true;
                }

                //当读到最后一天的最后一条数据  返回真
				if (day_tmp == day_num && requests_all->back().day_request.size() == everyday_num.back())
				{
					read_all_finished = true;
					return true;
				}
            }
            lineStream.clear();
            if(debug) cout<<"\n";
            return false;
        }
    }
    //~DataHandling();





    bool openFile(const char *filePath)
    {
        ifstream infile(filePath,std::ios::in);
        string tmp_line;
        
        cout <<"path: " <<filePath << "\n";
        
        if (!infile.fail())
        {          
            while(getline(infile, tmp_line ))  //逐行读取
            {
				if (tmp_line.length() > 0)
					if (dealLineData(tmp_line))
						break;
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
};
#endif