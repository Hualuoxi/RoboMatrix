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
public:
    unordered_map <string, ServersData> servers;  //所有服务器
    unordered_map <string, VMData> vms;   //所有虚拟机
    vector<DayRequestData> *requests_all;  //所有天的请求数据
	int servers_num, vms_num, day_num, k_num; //服务器型号数量  虚拟机型号数量 总天数 提前知晓的天数
	bool readed_kday_reqs=false;  //是否读完k天的请求
	bool day_read_finished= false;     //是否读取完当天数据
	bool read_all_finished = false;    //是否读取完所有天数据
    DataHandling(bool _debug=true)
    {
        debug = _debug;
    };
    //是否读取完所有数据
    bool dealLineData(string tmp_line)
    {
        if(debug) cout<< i <<"\n"; i=i+1;
		day_read_finished = false;
        if(tmp_line.data()[0] != '(' ) //
        {
            if(num == 1) servers_num = stoi(tmp_line);
            if(num == 2) vms_num = stoi(tmp_line);
            if(num == 3) {
				string tep_word;
				stringstream words(tmp_line);
				int i = 2;
				while (getline(words, tep_word, ' '))
				{
					if (i == 2){
						day_num = stoi(tep_word);
						i--;
					}
					else
						k_num = stoi(tep_word);
				}
                
                requests_all = new vector<DayRequestData>(day_num);
            }
            if(num > 3 ) 
            {
                everyday_num.push_back( stoi(tmp_line) );
                day_tmp++;
				if (day_tmp >= k_num)  
					readed_kday_reqs = true;
            }
            num+=1;
            if(debug) cout<<"num:  "<<stoi(tmp_line) <<"\n";
            return false;
        }
        else
        {
            p = tmp_line.begin();
            tmp_line.erase(p);  
            p = tmp_line.end(); p--;
            tmp_line.erase(p); 
            stringstream words(tmp_line);
            string tep_word;
            if(debug) cout <<"line:  " <<tmp_line <<endl;
            ser_tmp = 5; vm_tmp = 4;req_tmp=3;
            while(getline(words, tep_word, ','))//以,为分隔符，读取数据
            {
                if(num == 2) serversDeal(tep_word);
                if(num == 3) vmsDeal(tep_word);
                if(num>3)  requestsDeal(tep_word);
            }

            if(num ==2) 
            {
                servers[servers_data.server_type] = servers_data;
            }
            if(num == 3)
            {
                // pair<string,VMData>tmp (vms_data.vm_type,vms_data);
                // vms.insert(tmp);
                vms[vms_data.vm_type] = vms_data;
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
					day_read_finished = true;
                //当读到最后一天的最后一条数据  返回真
				if (day_tmp == day_num && requests_all->back().day_request.size() == everyday_num.back())
				{
					read_all_finished = true;
					return true;
				}
            }

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


    void stdCout( )
    {

    }

private:
    
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
            vm_tmp--;
            if(debug) cout << vms_data.vm_type<<"  ";
        }
        else if(vm_tmp == 3){
            vms_data.cpu = stoi(_word);
            vm_tmp--;
            if(debug) cout << vms_data.cpu<<"  ";
        }
        else if(vm_tmp == 2){
            vms_data.memory = stoi(_word);
            vm_tmp--;
            if(debug) cout << vms_data.memory<<"  ";
        }
         else if(vm_tmp == 1){
            vms_data.node = stoi(_word);
            vm_tmp--;
            if(debug) cout << vms_data.node<<"  ";
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
                _word.erase(0,_word.find_first_not_of(" "));  //去年首部空格
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