#ifndef STRATEGY
#define STRATEGY

#include "data_handling.hpp"
#include <list>
#include <algorithm>

//单个服务器
struct OwnServer
{
	int id;    //记录当前服务器id  从0开始计数
	int day_num;  //运行天数
	ServersData ser;   //服务器类型
	int cpu_A_left;    //A节点剩余的cpu
	int memory_A_left; //A节点剩余的memory
	int cpu_B_left;    //B节点剩余的cpu
	int memory_B_left;  //B节点剩余的memory

	OwnServer(int _id, ServersData _ser)
	{
		id = _id;
		ser = _ser;
		cpu_A_left = ser.cpu / 2;
		memory_A_left = ser.memory / 2;
		cpu_B_left = ser.cpu / 2;
		memory_B_left = ser.memory / 2;
	}

	//将虚拟机放入服务器  _a_b放入的节点 （-1 双节点    0 a  1 b）
	bool insertVM(VMData _vm_data ,int *_a_b)
	{
		if (_vm_data.node == 1)  //双节点 
		{
			*_a_b = -1;
			if (cpu_A_left > _vm_data.cpu / 2 && cpu_B_left > _vm_data.cpu / 2  && memory_A_left > _vm_data.memory/2 && memory_B_left> _vm_data.memory/2)
			{
				cpu_A_left -= _vm_data.cpu / 2;
				memory_A_left -=  _vm_data.memory / 2;
				cpu_B_left -= _vm_data.cpu / 2;
				memory_B_left -= _vm_data.memory / 2;
				return true;
			}
			else
				return false;
		}
		else  // 单节点
		{
			if (cpu_A_left > _vm_data.cpu  &&  memory_A_left > _vm_data.memory)
			{
				*_a_b = 0;
				cpu_A_left -= _vm_data.cpu ;
				memory_A_left -= _vm_data.memory;
				return true;
			}
			else if(cpu_B_left > _vm_data.cpu && memory_B_left > _vm_data.memory)
			{
				*_a_b = 1;
				cpu_B_left -= _vm_data.cpu;
				memory_B_left -= _vm_data.memory;
				return true;
			}
			else
				return false;
		}
	}
	//去掉虚拟机  _a_b放入的节点 （-1 双节点    0 a  1 b）
	void removeVM(VMData _vm_data,int _a_b)
	{
		if (_a_b ==-1)  //双节点 
		{
			cpu_A_left += _vm_data.cpu / 2;
			memory_A_left += _vm_data.memory / 2;
			cpu_B_left += _vm_data.cpu / 2;
			memory_B_left += _vm_data.memory / 2;
		}
		else if (_a_b == 0)
		{
			cpu_A_left += _vm_data.cpu;
			memory_A_left += _vm_data.memory;
		}
		else if (_a_b == 1)
		{
			cpu_B_left += _vm_data.cpu;
			memory_B_left += _vm_data.memory;
		}
	}
};

//购买单个服务器的数据
struct PurSerData
{
	string type;  
	int num; 
}; 

//虚拟机与对应的服务器
struct VM2Server
{
	VMData vm;    //虚拟机类型
	OwnServer *own_ser;  //指向的服务器
	int a_b;   //-1 双节点    0 放在a节点  1 b节点  
};

//拥有的所有服务器
struct AllServers
{
	//正在使用的服务器
	list<OwnServer> using_ser;
	//空闲的服务器
	//vector<OwnServer> left_ser;  //空闲服务器怎么判断？

	//<天数,<型号 数量> >  天数从0开始
	unordered_map<int,unordered_map<string, PurSerData>> pur_sers;

	unsigned int num=0;   //计数  每次添加服务器就加1  从0开始


	//添加服务器 _ser_dat：类型  day_id：第多少天
	void addSer(ServersData _ser_dat , int _day_id)
	{
		OwnServer ser_state(num, _ser_dat);
		num += 1;
		using_ser.push_back(ser_state);

		if (pur_sers.find(_day_id) == pur_sers.end())  //如果没有当天数据
		{ 
			pur_sers[_day_id][_ser_dat.server_type].type = _ser_dat.server_type;
			pur_sers[_day_id][_ser_dat.server_type].num = 1;
		}
		else if(pur_sers[_day_id].find(_ser_dat.server_type) == pur_sers[_day_id].end())  //当天没有该服务器的数据
		{
			pur_sers[_day_id][_ser_dat.server_type].type = _ser_dat.server_type;
			pur_sers[_day_id][_ser_dat.server_type].num += 1;
		}
		else //该服务器num+1
		{
			pur_sers[_day_id][_ser_dat.server_type].num += 1;
		}
	}


};




class Strategy
{
public:
	Strategy(DataHandling *_data_hand) 
	{ 
		data_hand = _data_hand; 
		for (auto it = _data_hand->servers.begin(); it != _data_hand->servers.end(); ++it)
		{
			//cout << it->first << endl;
			if (max_cpu_ser.cpu < _data_hand->servers.at(it->first).cpu)
				max_cpu_ser = _data_hand->servers.at(it->first);
			if (max_mem_ser.memory < _data_hand->servers.at(it->first).memory)
				max_mem_ser = _data_hand->servers.at(it->first);

			if(min_hardcost_ser.hardware_cost ==0) //初值
				min_hardcost_ser = _data_hand->servers.at(it->first);
			if (min_hardcost_ser.hardware_cost > _data_hand->servers.at(it->first).hardware_cost)
				min_hardcost_ser = _data_hand->servers.at(it->first);
		}
	};

	void dealDayReq(DayRequestData *dat_req ,int _day_id)
	{
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			addVm(dat_req->add_req.at(i), _day_id);
		}
		for (int i = 0; i < dat_req->del_req.size(); i++)
		{
			delVm(dat_req->del_req.at(i));
		}

	}


	//添加虚拟机 及 放置到对应服务器
	void addVm(RequestData _req ,int _day_id)
	{
		bool inset_success = false;
		
		//判断使用中服务器中是否有空闲位置
		for (auto it = own_ser.using_ser.begin(); it != own_ser.using_ser.end(); it++)
		{
			inset_success = it->insertVM( data_hand->vms.at(_req.vm_type), &vms_ser[_req.id].a_b);
			if (inset_success)
			{
				vms_ser[_req.id].own_ser = &*it;
				break;
			}
		}

		if (!inset_success)  //没有成功加入虚拟机  空间不足  申请新的服务器
		{
			own_ser.addSer(max_cpu_ser , _day_id);  //先只考虑一种
			own_ser.using_ser.back().insertVM(data_hand->vms.at(_req.vm_type) , &vms_ser[_req.id].a_b);
			vms_ser[_req.id].own_ser = &own_ser.using_ser.back();            
		}
		vms_ser[_req.id].vm = data_hand->vms.at(_req.vm_type);

	}

	//删除虚拟机
	void delVm(RequestData _req)
	{
		vms_ser[_req.id].own_ser->removeVM(vms_ser[_req.id].vm, vms_ser[_req.id].a_b);
		vms_ser.erase(_req.id);

	}
	
	void coutDayMsg(int _day_id)
	{
		//购买型号的数量
		if (own_ser.pur_sers.find(_day_id) != own_ser.pur_sers.end())  //如果当天购买不为空
		{
			cout << "(purchase, " << own_ser.pur_sers.at(_day_id).size() << ")\n";
			for (auto it = own_ser.pur_sers.at(_day_id).begin(); it != own_ser.pur_sers.at(_day_id).end(); ++it)
			{
				//if (max_cpu_ser.cpu < _data_hand->servers.at(it->first).cpu)
				//型号及其对应购买数量
				cout << "(" << own_ser.pur_sers.at(_day_id).at(it->first).type << ", " << own_ser.pur_sers.at(_day_id).at(it->first).num << ")\n";

			}
		}
		else
		{
			cout << "(purchase, 0)\n";
		}

		//迁移数量
		cout << "(migration, " << 0 << ")\n";


		//虚拟机部署到服务器 id和节点。
		for (int i = 0; i < data_hand->requests_all->at(_day_id).add_req.size(); i++)
		{
			if (vms_ser.find(data_hand->requests_all->at(_day_id).add_req.at(i).id) != vms_ser.end())
			{
				cout << "(" << vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).own_ser->id;
				if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == -1)
					cout << ")\n";
				else if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == 0)
					cout << ", A)\n";
				else if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == 1)
					cout << ", B)\n";
			}
		}
	}

private:
	DataHandling *data_hand;
	AllServers own_ser;
    unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器
	ServersData max_cpu_ser, max_mem_ser, min_hardcost_ser;
};



#endif