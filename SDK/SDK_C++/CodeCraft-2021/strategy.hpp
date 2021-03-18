#ifndef STRATEGY
#define STRATEGY

#include "data_handling.hpp"
#include <list>
#include "math.h"
#include <cmath>
//#include <algorithm>

//单个服务器
struct OwnServer
{
	int id;    //记录当前服务器id  从0开始计数
	int day_num;  //运行天数  todo
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
	//打印利用率
	void coutUsage()
	{
		cout<< "CPU: " << 1 - (cpu_A_left + cpu_B_left) / (float)ser.cpu<< "  mem: " << 1 - (memory_A_left + memory_B_left) / (float)ser.memory <<"\n";
	}
};

//购买单个服务器的数据
struct PurSerData
{
	string type;   //类型
	int num;     //数量
}; 

//虚拟机与对应的服务器
struct VM2Server
{
	VMData vm;    //虚拟机类型
	OwnServer *own_ser;  //指向的服务器
	int a_b;   //-1 双节点    0 放在a节点  1 b节点  
	bool dealed; //是否加入了服务器
	vector<VM2Server*> matchs_k; //匹配的型号 cpu_mem 按匹配度排序
	bool matched;  //是否匹配过
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
		max_cpu_ser.cpu = max_mem_ser.memory = 0;
		for (auto it = _data_hand->servers.begin(); it != _data_hand->servers.end(); ++it)
		{
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
		max_cpu =max_mem= 0;
		//先将请求数据转换  转换为VM2Server格式
		for(int i = 0; i < dat_req->add_req.size(); i++)
		{
			vms_ser[dat_req->add_req.at(i).id].dealed = false;
			vms_ser[dat_req->add_req.at(i).id].matched = false;
			vms_ser[dat_req->add_req.at(i).id].vm = data_hand->vms.at(dat_req->add_req.at(i).vm_type);
			if (data_hand->vms.at(dat_req->add_req.at(i).vm_type).node == 0) //单节点  cpu memory 按两倍算
			{
				if (max_cpu < 2 * data_hand->vms.at(dat_req->add_req.at(i).vm_type).cpu)
					max_cpu = 2 * data_hand->vms.at(dat_req->add_req.at(i).vm_type).cpu;
				if (max_mem < 2 * data_hand->vms.at(dat_req->add_req.at(i).vm_type).memory)
					max_mem = 2 * data_hand->vms.at(dat_req->add_req.at(i).vm_type).memory;
			}
			else
			{
				if (max_cpu < data_hand->vms.at(dat_req->add_req.at(i).vm_type).cpu)
					max_cpu = data_hand->vms.at(dat_req->add_req.at(i).vm_type).cpu;
				if (max_mem < data_hand->vms.at(dat_req->add_req.at(i).vm_type).memory)
					max_mem = data_hand->vms.at(dat_req->add_req.at(i).vm_type).memory;
			}
			
		}
		vector<VM2Server*> vms_node_s, vms_node_d;  //单节点  双节点
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			if (vms_ser[dat_req->add_req.at(i).id].vm.node == 1)
				vms_node_d.push_back(&vms_ser[dat_req->add_req.at(i).id]);
			else
				vms_node_s.push_back(&vms_ser[dat_req->add_req.at(i).id]);
		}
		//排序
		bubble(vms_node_d);
		bubble(vms_node_s);
		//先将虚拟机放入已有服务器  先双后单
		for (int j = 0; j < vms_node_d.size(); j++)
		{
			addVm2Ser(vms_node_d.at(j), false, _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			addVm2Ser(vms_node_s.at(j), false, _day_id);
		}

		//day_ser_select = max_cpu_ser;
		//选择服务器
		selectSer(dat_req);     //服务器选择存在问题
		//查找与每天相匹配的型号 
		findMatchsDay(dat_req);
		//将未处理请求加入服务器
	    for (int j = 0; j < vms_node_d.size(); j++)
		{
			if(vms_node_d.at(j)->dealed ==false)
				addMatchVms2ser(vms_node_d.at(j), _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			if (vms_node_s.at(j)->dealed == false)
				addMatchVms2ser(vms_node_s.at(j), _day_id);
		}
		
		//处理删除请求
		for (int i = 0; i < dat_req->del_req.size(); i++)
		{
			delVm(dat_req->del_req.at(i));
		}
	}

	//根据每天没有处理过的虚拟机数据总和 选择服务器
	void selectSer(DayRequestData *dat_req)
	{
		float cpu_sum = 0, mem_sum = 0;
		int num=0, cost=0;
		string _type;
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			if (vms_ser.at(dat_req->add_req.at(i).id).dealed == false)
			{
				cpu_sum += vms_ser.at(dat_req->add_req.at(i).id).vm.cpu;
				mem_sum += vms_ser.at(dat_req->add_req.at(i).id).vm.memory;
			}
		}

		//根据每日成本最小查找
		for (auto it = data_hand->servers.begin(); it != data_hand->servers.end(); ++it)
		{
			int c_num = ceil( cpu_sum / data_hand->servers.at(it->first).cpu);  //向上取整
			int m_num = ceil( mem_sum / data_hand->servers.at(it->first).memory);
			int _num = (c_num>m_num)? c_num: m_num;
			int it_cost = _num * data_hand->servers.at(it->first).hardware_cost;

			//申请的型号需要满足cpu与mem都大于当前申请的最大值
			if (data_hand->servers.at(it->first).cpu > max_cpu && data_hand->servers.at(it->first).memory > max_mem)
			{
				if (cost == 0)  //未初始化
				{
					cost = it_cost;
					_type = data_hand->servers.at(it->first).server_type;
					num = _num;
				}
				else if (cost > it_cost)
				{
					cost = it_cost;
					_type = data_hand->servers.at(it->first).server_type;
					num = _num;
				}
			}
		}

		//根据每日比例查找
		//float cpu_mem, dk, d_val;
		//cpu_mem = cpu_sum / mem_sum;
		//dk = abs(max_cpu_ser.cpu / max_cpu_ser.memory - cpu_mem);  //
		//_type = max_cpu_ser.server_type;
		////查找最小差值
		//for (auto it = data_hand->servers.begin(); it != data_hand->servers.end(); ++it)
		//{
		//	d_val = abs(data_hand->servers.at(it->first).cpu / data_hand->servers.at(it->first).memory - cpu_mem);
		//	if (dk > d_val)
		//	{
		//		//申请的型号需要满足cpu与mem都大于当前申请的最大值2倍
		//		if (data_hand->servers.at(it->first).cpu >= max_cpu*2 && data_hand->servers.at(it->first).memory >= max_mem*2)
		//		{
		//			dk = d_val;
		//			_type = data_hand->servers.at(it->first).server_type;
		//		}
		//	}
		//}

		day_ser_select = data_hand->servers.at(_type);
	}

	//查找与每天相匹配的型号 
	//cpu/mem是否与服务器的匹配  或者几个vm的和的cpu/mem与服务器匹配
	void findMatchsDay(DayRequestData *dat_req)
	{
		float _day_k = day_ser_select.cpu / day_ser_select.memory;
		int id,id_left;
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			id = dat_req->add_req.at(i).id;
			if (vms_ser.at(id).dealed == false && vms_ser.at(id).matched == false) //未加入服务器 并且未匹配
			{
				float cpu_mem = vms_ser.at(id).vm.cpu / vms_ser.at(id).vm.memory;
				//该型号与服务器分布相似
				if (abs(cpu_mem - _day_k) < 0.1)
				{
					vms_ser.at(id).matched = true;
					continue; //不查找匹配型号
				}
				for (int j = 0; j < dat_req->add_req.size(); j++)
				{
					id_left = dat_req->add_req.at(j).id;
					//不是本身  未加入服务器 并且未匹配 并且节点类型相同
					if (id != id_left && vms_ser.at(id_left).dealed == false && vms_ser.at(id_left).matched == false && vms_ser.at(id).vm.node == vms_ser.at(id_left).vm.node)  
					{
						//先只考虑两个组合
						float k = (vms_ser.at(id).vm.cpu + vms_ser.at(id_left).vm.cpu) / (vms_ser.at(id).vm.memory + vms_ser.at(id_left).vm.memory);
						if (abs(k - _day_k) < 0.1)
						{
							vms_ser.at(id).matchs_k.push_back(&vms_ser.at(id_left));  //互相放入对方列表
							vms_ser.at(id_left).matchs_k.push_back(&vms_ser.at(id));
							vms_ser.at(id_left).matched = true;
							vms_ser.at(id).matched = true;
							break;
						}
					}
				}
			}
		}
	}


	//添加虚拟机到服务器 add_new_ser:如果没有空间，是否申请新服务器
	void addVm2Ser(VM2Server *_vm2ser,bool add_new_ser, int _day_id)
	{
		bool inset_success = false;
		
		//判断使用中服务器中是否有空闲位置
		for (auto it = own_sers.using_ser.begin(); it != own_sers.using_ser.end(); it++)
		{
			inset_success = it->insertVM( _vm2ser->vm, &_vm2ser->a_b);
			if (inset_success)
			{
				_vm2ser->own_ser = &*it;
				_vm2ser->dealed = true;
				break;
			}
		}
		if (!add_new_ser) return;

		if (!inset_success)  //没有成功加入虚拟机  空间不足  申请新的服务器
		{
			bool inset_success2 = false;
			own_sers.addSer(day_ser_select, _day_id);  //添加服务器
			inset_success2 = own_sers.using_ser.back().insertVM(_vm2ser->vm, &_vm2ser->a_b);
			_vm2ser->own_ser = &own_sers.using_ser.back();
			_vm2ser->dealed = true;
		}
	}
	//将虚拟机添加到尾部服务器
	void addVmsSerBack(VM2Server *_vm2ser, int _day_id)
	{
		bool inset_success = false;
		inset_success = own_sers.using_ser.back().insertVM(_vm2ser->vm, &_vm2ser->a_b);
		if (inset_success)
		{
			_vm2ser->own_ser = &own_sers.using_ser.back();
			_vm2ser->dealed = true;
		}
		else
		{
			own_sers.addSer(day_ser_select, _day_id);  //添加服务器
			own_sers.using_ser.back().insertVM(_vm2ser->vm, &_vm2ser->a_b);
			_vm2ser->own_ser = &own_sers.using_ser.back();
			_vm2ser->dealed = true;
		}
	}

	//同时添加匹配的虚拟机到已有服务器  没有空间则申请新的服务器
	void addMatchVms2ser(VM2Server *_vm2ser,int _day_id)
	{
		if (_vm2ser->dealed == true) return;  //如果已经处理过

		addVm2Ser(_vm2ser, true, _day_id);
		if (_vm2ser->matchs_k.size() != 0)  //有匹配项 
		{
			if (_vm2ser->matchs_k.at(0)->dealed == false) //并且未处理
			{
				//addVm2Ser(_vm2ser->matchs_k.at(0), true, _day_id);
				addVmsSerBack(_vm2ser->matchs_k.at(0), _day_id);
			}
		}
	}

	//删除虚拟机
	void delVm(RequestData _req)
	{
		vms_ser[_req.id].own_ser->removeVM(vms_ser[_req.id].vm, vms_ser[_req.id].a_b);
		vms_ser.erase(_req.id);

	}

	//冒泡排序  按cpu与mem的  由大到小排序
	void bubble(vector<VM2Server*> &vms)
	{
		int len = vms.size();
		for (int i = 0; i < len; i++) {//控制总的趟数
			for (int j = 1; j < len - i; ++j) {//一次冒泡排序的结果
				if ((vms[j - 1]->vm.cpu * vms[j - 1]->vm.memory) < (vms[j]->vm.cpu * vms[j]->vm.memory))
					swap(vms[j - 1], vms[j]);
			}
		}
	}

	//输出每日信息
	void coutDayMsg(int _day_id)
	{
		//购买型号的数量
		if (own_sers.pur_sers.find(_day_id) != own_sers.pur_sers.end())  //如果当天购买不为空
		{
			cout << "(purchase, " << own_sers.pur_sers.at(_day_id).size() << ")\n";
			for (auto it = own_sers.pur_sers.at(_day_id).begin(); it != own_sers.pur_sers.at(_day_id).end(); ++it)
			{
				//if (max_cpu_ser.cpu < _data_hand->servers.at(it->first).cpu)
				//型号及其对应购买数量
				cout << "(" << own_sers.pur_sers.at(_day_id).at(it->first).type << ", " << own_sers.pur_sers.at(_day_id).at(it->first).num << ")\n";

			}
		}
		else
		{
			cout << "(purchase, 0)\n";
		}

		//迁移数量
		cout << "(migration, " << 0 << ")\n";

		//虚拟机部署到服务器 id和节点。
		for (int i = 0; i < data_hand->requests_all->at(_day_id).add_req.size(); i++)  //根据请求id 顺序输出
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

	void coutAllSersUsage()
	{
		for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		{
			iter->coutUsage();
		}
	}
	
	
private:
	DataHandling *data_hand;
	AllServers own_sers;  //拥有的所有服务器              
    unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器  <id,对应服务器>
	ServersData max_cpu_ser, max_mem_ser, min_hardcost_ser;
	ServersData day_ser_select;    //当天选择的服务器类型
	int max_cpu, max_mem;  //每天请求中最大cpu与最大mem
};



#endif