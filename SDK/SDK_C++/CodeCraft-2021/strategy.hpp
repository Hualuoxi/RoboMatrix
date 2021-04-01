#ifndef STRATEGY
#define STRATEGY

#include "data_handling.hpp"
#include <list>
#include <cmath>
#include <time.h>

struct OwnServer;  //声明

//虚拟机与对应的服务器
struct VM2Server
{
	int vm_id;      //虚拟机id
	VMData vm;    //虚拟机类型
	OwnServer *own_ser;  //指向的服务器
	int a_b;   //-1 双节点    0 放在a节点  1 b节点  
	int a_b_mig;  //迁移前的节点位置   迁移后节点位置会变，从原服务器删除时需要原来放置的节点位置
	bool dealed; //是否加入了服务器
	vector<VM2Server*> matchs_k; //匹配的型号 相同类型的节点才会匹配
	bool matched;  //是否匹配过
};

//单个服务器
struct OwnServer
{
	int id;    //记录当前服务器id  从0开始计数
	ServersData ser;   //服务器类型
	float cpu_A_left;    //A节点剩余的cpu
	float memory_A_left; //A节点剩余的memory
	float cpu_B_left;    //B节点剩余的cpu
	float memory_B_left;  //B节点剩余的memory
	float usage_cpu;   //cpu总利用率
	float usage_mem;   //mem总利用率
	float cost_all;   //总成本
	int day_start;  //运行起始天  
	int use_days;    //使用总天数
	bool unused;
	list<VM2Server*> vms;  //存放的虚拟机
	OwnServer(int _id, ServersData _ser)
	{
		id = _id;
		ser = _ser;
		cpu_A_left = ser.cpu / 2.;
		memory_A_left = ser.memory / 2.;
		cpu_B_left = ser.cpu / 2.;
		memory_B_left = ser.memory / 2.;
		usage_cpu = usage_mem = 0;
		day_start = use_days = 0;
		unused = false;
	}

	//将虚拟机放入服务器  （-1 双节点    0 a  1 b）
	bool insertVM(VM2Server *_vm, int _day_id)
	{
		if (_vm->vm.node == 1)  //双节点 
		{
			_vm->a_b = -1;
			if (cpu_A_left >= _vm->vm.cpu / 2. && cpu_B_left >= _vm->vm.cpu / 2.  && memory_A_left >= _vm->vm.memory / 2. && memory_B_left >= _vm->vm.memory / 2.)
			{
				cpu_A_left -= _vm->vm.cpu / 2.;
				memory_A_left -= _vm->vm.memory / 2.;
				cpu_B_left -= _vm->vm.cpu / 2.;
				memory_B_left -= _vm->vm.memory / 2.;
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
				if (day_start == 0)  day_start = _day_id; //记录起始天
				unused = false;
				vms.push_back(_vm);
				return true;
			}
			else
				return false;
		}
		else  // 单节点
		{
			if (cpu_A_left >= _vm->vm.cpu  &&  memory_A_left >= _vm->vm.memory)
			{
				_vm->a_b = 0;
				cpu_A_left -= _vm->vm.cpu;
				memory_A_left -= _vm->vm.memory;
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
				if (day_start == 0)  day_start = _day_id; //记录起始天
				unused = false;
				vms.push_back(_vm);
				return true;
			}
			else if (cpu_B_left >= _vm->vm.cpu && memory_B_left >= _vm->vm.memory)
			{
				_vm->a_b = 1;
				cpu_B_left -= _vm->vm.cpu;
				memory_B_left -= _vm->vm.memory;
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
				if (day_start == 0)
					day_start = _day_id; //记录起始天
				unused = false;
				vms.push_back(_vm);
				return true;
			}
			else
				return false;
		}
	}
	//去掉虚拟机  
	void removeVM(VM2Server *_vm, int _a_b, int _day_id)
	{
		if (_a_b == -1)  //双节点 
		{
			cpu_A_left += _vm->vm.cpu / 2.;
			memory_A_left += _vm->vm.memory / 2.;
			cpu_B_left += _vm->vm.cpu / 2.;
			memory_B_left += _vm->vm.memory / 2.;
		}
		else if (_a_b == 0)
		{
			cpu_A_left += _vm->vm.cpu;
			memory_A_left += _vm->vm.memory;
		}
		else if (_a_b == 1)
		{
			cpu_B_left += _vm->vm.cpu;
			memory_B_left += _vm->vm.memory;
		}
		vms.remove(_vm);

		usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
		usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
		if (usage_cpu == 0 && usage_mem == 0)  //空闲的天次
		{
			use_days += (_day_id - day_start + 1);  //已经运行总天数
			day_start = 0;
			unused = true;
		}
	}

	//打印利用率
	void coutUsage()
	{
		cout << "CPU: " << usage_cpu << "  mem: " << usage_mem << "\n";
	}
	//计算总成本 
	void calCosts(int _day_id)
	{
		if (unused == false)
			use_days += (_day_id - day_start + 1);

		cost_all = ser.hardware_cost + ser.energy_day * use_days;
	}
};

//购买单个服务器的数据
struct PurSerData
{
	string type;   //类型
	int num;     //数量
};

//删除请求
struct DelMsg
{
	int id;
	bool dealed;
};
//迁移信息
struct MigrationMsg
{
	int vm_id;  //虚拟机id
	int aim_id;  //目标服务器id
	int a_b;     //放到目标服务器的节点
};

bool myCompare(OwnServer &p1, OwnServer &p2)
{
	return (p1.usage_cpu + p1.usage_mem) > (p2.usage_cpu + p2.usage_mem);		//按二者和排序
}

bool myCompareByenergy(OwnServer &p1, OwnServer &p2)
{
	return  (p1.ser.energy_day/(p1.ser.cpu+p1.ser.memory)/(p1.usage_cpu + p1.usage_mem) < p2.ser.energy_day/(p2.ser.cpu+p2.ser.memory)/(p2.usage_cpu + p2.usage_mem));		//按二者和排序
}

bool myCompareByUsageRat(OwnServer &p1, OwnServer &p2)
{
	return  fabs(p1.usage_cpu - p1.usage_mem) > fabs(p2.usage_cpu - p2.usage_mem);		//按二者和排序
}

//拥有的所有服务器
struct AllServers
{
	//正在使用的服务器  只能使用list
	list<OwnServer> using_ser;

	//<天数,<型号 数量> >  天数从0开始
	unordered_map<int, unordered_map<string, PurSerData>> pur_sers;

	unsigned int num = 0;   //计数  每次添加服务器就加1  从0开始

	//添加服务器 _ser_dat：类型  day_id：第多少天
	void addSer(ServersData _ser_dat, int _day_id)
	{
		OwnServer ser_state(num, _ser_dat);
		num += 1;
		using_ser.push_back(ser_state);

		if (pur_sers.find(_day_id) == pur_sers.end())  //如果没有当天数据
		{
			pur_sers[_day_id][_ser_dat.server_type].type = _ser_dat.server_type;
			pur_sers[_day_id][_ser_dat.server_type].num = 1;
		}
		else if (pur_sers[_day_id].find(_ser_dat.server_type) == pur_sers[_day_id].end())  //当天没有该服务器的数据
		{
			pur_sers[_day_id][_ser_dat.server_type].type = _ser_dat.server_type;
			pur_sers[_day_id][_ser_dat.server_type].num += 1;
		}
		else //该服务器num+1
		{
			pur_sers[_day_id][_ser_dat.server_type].num += 1;
		}
	}
	//排序
	void sort()
	{
		// using_ser.sort(myCompareByenergy);
		 using_ser.sort(myCompare);
	}
	void sortbyUsageRat()
	{
		 using_ser.sort(myCompareByUsageRat);
	}

};

class Strategy
{
public:
	DataHandling *data_hand;
	Strategy(DataHandling *_data_hand)
	{
		data_hand = _data_hand;
	};

	void dealDayReq(DayRequestData *dat_req, int _day_id)
	{
		if(all_day_num==0) all_day_num = data_hand->requests_all->size() - 1;  //从0开始 

		mig_msgs_day.clear();  //清除上一天数据
		mig_num_day = 0;
		unordered_map<int, DelMsg> del_msg_day;   //每日的删除请求
		DelMsg msg;
		for (int i = 0; i < dat_req->del_req.size(); i++)
		{
			msg.id = dat_req->del_req.at(i).id;
			msg.dealed = false;
			del_msg_day[msg.id] = msg;
		}
		float del_rat = (float)(data_hand->requests_all->at(_day_id).del_req.size()) /(float)(data_hand->requests_all->at(_day_id).day_request.size());
		if(del_rat < 0.7f)
		{
			own_sers.sortbyUsageRat();
			migLowCPU2Mem(&mig_msgs_day, _day_id,1.1);
			own_sers.sort();
			migration(&mig_msgs_day, _day_id);
		}
		else
		{
			migration(&mig_msgs_day, _day_id);
			migLowCPU2Mem(&mig_msgs_day, _day_id,1.13);
		}

		// migration(&mig_msgs_day, _day_id);
		// migDelVm2empSer(&mig_msgs_day,_day_id,del_msg_day);
		
		vector<VM2Server*> vms_node_s, vms_node_d;  //单节点  双节点
		//先将请求数据转换  转换为VM2Server格式
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			//int id_tmp = dat_req->add_req.at(i).id;
			vms_ser[dat_req->add_req.at(i).id].vm_id =dat_req->add_req.at(i).id;
			vms_ser[dat_req->add_req.at(i).id].dealed = false;
			vms_ser[dat_req->add_req.at(i).id].matched = false;
			vms_ser[dat_req->add_req.at(i).id].vm = data_hand->vms.at(dat_req->add_req.at(i).vm_type);
			if (vms_ser[dat_req->add_req.at(i).id].vm.node == 1)
				vms_node_d.push_back(&vms_ser[dat_req->add_req.at(i).id]);
			else
				vms_node_s.push_back(&vms_ser[dat_req->add_req.at(i).id]);

		}

		//删除的消息转化为 DelMsg 格式

		//排序
		quickSort(vms_node_d,0,vms_node_d.size()-1);
		quickSort(vms_node_s,0,vms_node_s.size()-1);
		// bubble(vms_node_d);
		// bubble(vms_node_s);
		//先将虚拟机放入已有服务器  先双后单
		for (int j = 0; j < vms_node_d.size(); j++)
		{
			addVm2Ser(vms_node_d.at(j), false, _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			addVm2Ser(vms_node_s.at(j), false, _day_id);
		}

		selectSer(dat_req, _day_id);

		//将剩下的虚拟机放入服务器  先双后单
		for (int j = 0; j < vms_node_d.size(); j++)
		{
			if (vms_node_d.at(j)->dealed == false)
				addVm2Ser(vms_node_d.at(j), true, _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			if (vms_node_s.at(j)->dealed == false)
				addVm2Ser(vms_node_s.at(j), true, _day_id);
		}
		// //处理剩下的删除请求
		for (auto it = del_msg_day.begin(); it != del_msg_day.end(); ++it)
		{
			if (del_msg_day.at(it->first).dealed == false)
				delVm(&del_msg_day.at(it->first), _day_id);
		}

		//服务器排序
		own_sers.sort();
	}

	void migration(vector<MigrationMsg> *mig_msg, int _day_id)
	{
		unordered_map<int, OwnServer*> mig_sers; //迁移的服务器或者空闲的 之后就不往里面迁移了
		int mig_num = 3 * vms_ser.size() / 100;
		if (own_sers.using_ser.size() == 0) return;

		for (auto it = --own_sers.using_ser.end(); it != own_sers.using_ser.begin(); it--)
		{
			if (mig_num_day >= mig_num)
				break;
			if (it->usage_cpu == 0 || it->usage_mem == 0)
			{
				mig_sers[it->id] = &*it;
				continue; //利用率为0就不考虑了
			}
			if (it->usage_cpu < 0.7 || it->usage_mem < 0.7) //利用率低  0.8 timeout
			{
				mig_sers[it->id] = &*it;
				vector<VM2Server*> vms_remove;
				for (auto it_vm = it->vms.begin(); it_vm != it->vms.end(); ++it_vm)
				{
					if (mig_num_day >= mig_num)
						break;
					bool inset_success = false;
					for (auto it2 = own_sers.using_ser.begin(); it2 != own_sers.using_ser.end(); ++it2)
					{
						if (mig_sers.find(it2->id) == mig_sers.end())   //不在迁移表中
						{
							if (mig_num_day >= mig_num)
								break;
							(*it_vm)->a_b_mig = (*it_vm)->a_b;
							inset_success = it2->insertVM(*it_vm, _day_id);
							if (inset_success)
							{
								MigrationMsg mig;
								mig.vm_id = (*it_vm)->vm_id;
								mig.aim_id = it2->id;
								mig.a_b = (*it_vm)->a_b;
								mig_msg->push_back(mig);

								(*it_vm)->own_ser = &*it2;
								//循环的时候不能删除元素  记录下来
								vms_remove.push_back(*it_vm);
								mig_num_day++;
								break;
							}
						}
					}
				}
				for (int i = 0; i < vms_remove.size(); i++)
				{
					it->removeVM(vms_remove.at(i), vms_remove.at(i)->a_b_mig, _day_id);
				}
			}
		}
		//cout << "mig_num: "<< mig_num << "  "<< vms_ser.size() <<"  "<< vms_ser.size()*5 /1000 <<"\n";
	}

	void migLowCPU2Mem(vector<MigrationMsg> *mig_msg, int _day_id , float rat)
	{
		vector<OwnServer*> sers_low_cpu, sers_low_mem;  //
		int mig_num = 3 * vms_ser.size() / 100;
		for (auto it1 = own_sers.using_ser.begin(); it1 != own_sers.using_ser.end(); ++it1)
		{
			if (it1->usage_mem / it1->usage_cpu > rat)
			{
				sers_low_cpu.push_back(&*it1);
			}
			if (it1->usage_cpu / it1->usage_mem > rat)
			{
				sers_low_mem.push_back(&*it1);
			}
			// if (it1->usage_mem > 0.85 && it1->usage_cpu > 0.5 && it1->usage_cpu < 0.7)
			// {
			// 	sers_low_cpu.push_back(&*it1);
			// }
			// if (it1->usage_cpu > 0.85 && it1->usage_mem > 0.5 && it1->usage_mem < 0.7)
			// {
			// 	sers_low_mem.push_back(&*it1);
			// }
		}
		if (sers_low_cpu.size() == 0 || sers_low_mem.size() == 0)  return;
		for (unsigned int i = 0; i < sers_low_cpu.size(); i++)
		{
			if (mig_num_day >= mig_num)
				break;
			vector<VM2Server*> vms_remove;
			for (auto it_vm = sers_low_cpu.at(i)->vms.begin(); it_vm != sers_low_cpu.at(i)->vms.end(); ++it_vm)
			{
				if (mig_num_day >= mig_num)
					break;
				bool inset_success = false;
				for (unsigned int j = 0; j < sers_low_mem.size(); j++)
				{
					if (mig_num_day >= mig_num)
						break;
					(*it_vm)->a_b_mig = (*it_vm)->a_b;
					inset_success = sers_low_mem.at(j)->insertVM(*it_vm, _day_id);
					if (inset_success)
					{
						MigrationMsg mig;
						mig.vm_id = (*it_vm)->vm_id;
						mig.aim_id = sers_low_mem.at(j)->id;
						mig.a_b = (*it_vm)->a_b;
						mig_msg->push_back(mig);

						(*it_vm)->own_ser = sers_low_mem.at(j);
						//循环的时候不能删除元素  记录下来
						vms_remove.push_back(*it_vm);
						mig_num_day++;
						break;
					}
				}
			}
			for (int s = 0; s < vms_remove.size(); s++)
			{
				sers_low_cpu.at(i)->removeVM(vms_remove.at(s), vms_remove.at(s)->a_b_mig, _day_id);
			}
		}

		for (unsigned int i = 0; i < sers_low_mem.size(); i++)
		{
			if (mig_num_day >= mig_num)
				break;
			vector<VM2Server*> vms_remove;
			for (auto it_vm = sers_low_mem.at(i)->vms.begin(); it_vm != sers_low_mem.at(i)->vms.end(); ++it_vm)
			{
				if (mig_num_day >= mig_num)
					break;
				bool inset_success = false;
				for (unsigned int j = 0; j < sers_low_cpu.size(); j++)
				{
					if (mig_num_day >= mig_num)
						break;
					(*it_vm)->a_b_mig = (*it_vm)->a_b;
					inset_success = sers_low_cpu.at(j)->insertVM(*it_vm, _day_id);
					if (inset_success)
					{
						MigrationMsg mig;
						mig.vm_id = (*it_vm)->vm_id;
						mig.aim_id = sers_low_cpu.at(j)->id;
						mig.a_b = (*it_vm)->a_b;
						mig_msg->push_back(mig);

						(*it_vm)->own_ser = sers_low_cpu.at(j);
						//循环的时候不能删除元素  记录下来
						vms_remove.push_back(*it_vm);
						mig_num_day++;
						break;
					}
				}
			}
			for (int s = 0; s < vms_remove.size(); s++)
			{
				sers_low_mem.at(i)->removeVM(vms_remove.at(s), vms_remove.at(s)->a_b_mig, _day_id);
			}
		}

	}

	void migDelVm2empSer(vector<MigrationMsg> *mig_msg,int _day_id,unordered_map<int,DelMsg> del_msg)
	{
		vector<OwnServer*> sers_emp;
		unordered_map<int,OwnServer*> sers_old;
		vector<VM2Server*> vms_remove;
		int mig_num = 3 * vms_ser.size() / 100;
		//找到没有使用的服务器作为目标服务器
		for (auto it = --own_sers.using_ser.end(); it!= own_sers.using_ser.begin(); --it)
		{
			if(it->usage_cpu ==0 || it->usage_mem ==0)
			{
				sers_emp.push_back(&*it);
			}
			else
				break;
		}
		if(sers_emp.size()==0) return;
		for (unsigned int j=0; j<sers_emp.size();j++)
		{		
			if (mig_num_day >= ( 5*vms_ser.size() /1000 ) )
				break;
			bool inset_success = false;
			if(del_msg.size() == 0) break;

			for (auto it_del_id = del_msg.begin(); it_del_id != del_msg.end(); ++it_del_id)
			{
				
				if (mig_num_day >= ( 5*vms_ser.size() /1000 ) )
					break;
				if(!it_del_id->second.dealed)
				{
					VM2Server* it_vm = &vms_ser.at(it_del_id->second.id);
					it_vm->a_b_mig = it_vm->a_b;
					inset_success = sers_emp.at(j)->insertVM(it_vm, _day_id);
					if (inset_success)
					{
						// cout <<"1 " <<endl;
						MigrationMsg mig;
						mig.vm_id = it_vm->vm_id;
						mig.aim_id = sers_emp.at(j)->id;
						mig.a_b = it_vm->a_b;
						mig_msg->push_back(mig);
						// cout <<"2 " <<endl;
						it_del_id->second.dealed=true;
						// cout <<"3 " <<endl;
						//循环的时候不能删除元素  记录下来
						sers_old.insert({it_del_id->second.id,it_vm->own_ser});
						vms_remove.push_back(it_vm);
						// cout <<"11 " <<sers_old[it_del_id->second.id]->ser.server_type<<" " << it_vm->own_ser->ser.server_type <<endl;
						it_vm->own_ser = sers_emp.at(j);
						// cout <<"22 " << sers_old[it_del_id->second.id]->ser.server_type<<" " << it_vm->own_ser->ser.server_type <<endl;
						mig_num_day++;
						
					}
					else
					{
						break;
					}
				}	
			}
		}
		for(int s=0 ;s < vms_remove.size() ;s++)
		{
			sers_old.at(vms_remove.at(s)->vm_id)->removeVM(vms_remove.at(s),vms_remove.at(s)->a_b_mig , _day_id);	
		}
		
	}

	//根据每天没有处理过的虚拟机数据总和 选择服务器
	void selectSer(DayRequestData *dat_req, int _day_id)
	{
		max_cpu = max_mem = 0;
		//查找当天没有处理过的虚拟机数据最大cpu
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			if (vms_ser[dat_req->add_req.at(i).id].dealed == false)
			{
				if (vms_ser[dat_req->add_req.at(i).id].vm.node == 0) //单节点cpu memory 按两倍算
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
		}

		float cpu_sum = 0, mem_sum = 0;
		int num = 0, cost = 0;
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
			int c_num = ceil(cpu_sum / data_hand->servers.at(it->first).cpu);  //向上取整
			int m_num = ceil(mem_sum / data_hand->servers.at(it->first).memory);
			int _num = (c_num > m_num) ? c_num : m_num;
			_num = ceil(_num / 0.8f);
			//数量 * 硬件成本加剩余天数的使用成本
			int it_cost = _num * (data_hand->servers.at(it->first).hardware_cost + (all_day_num - _day_id) * data_hand->servers.at(it->first).energy_day);

			//申请的型号需要满足cpu与mem都大于当前申请的最大值
			if (data_hand->servers.at(it->first).cpu >= max_cpu && data_hand->servers.at(it->first).memory >= max_mem)
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
		day_ser_select = data_hand->servers.at(_type);
	}

	//添加虚拟机到服务器 add_new_ser:如果没有空间，是否申请新服务器
	void addVm2Ser(VM2Server *_vm2ser, bool add_new_ser, int _day_id)
	{
		bool inset_success = false;

		//判断使用中服务器中是否有空闲位置
		for (auto it = own_sers.using_ser.begin(); it != own_sers.using_ser.end(); it++)
		{
			inset_success = it->insertVM(_vm2ser, _day_id);
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
			inset_success2 = own_sers.using_ser.back().insertVM(_vm2ser, _day_id);
			if (inset_success2)
			{
				_vm2ser->own_ser = &own_sers.using_ser.back();
				_vm2ser->dealed = true;
			}
		}
	}



	//将虚拟机添加到新增服务器
	bool addVmsDaySers(VM2Server *_vm2ser, vector<OwnServer> *_sers_day_add, ServersData *_ser_select, int _day_id)
	{
		bool inset_success = false;

		//判断服务器中是否有空闲位置
		for (auto it = _sers_day_add->begin(); it != _sers_day_add->end(); it++)
		{
			inset_success = it->insertVM(_vm2ser, _day_id);
			if (inset_success)
			{
				_vm2ser->own_ser = &*it;
				_vm2ser->dealed = true;
				return true;
			}
		}

		if (!inset_success)  //没有成功加入虚拟机  空间不足  申请新的服务器
		{
			bool inset_success2 = false;
			OwnServer ser_state(0, *_ser_select);
			_sers_day_add->push_back(ser_state);
			inset_success2 = _sers_day_add->back().insertVM(_vm2ser, _day_id);
			if (inset_success2)
			{
				_vm2ser->own_ser = &_sers_day_add->back();
				_vm2ser->dealed = true;
				return true;
			}
			else
				return false;
		}
	}

	//删除虚拟机
	void delVm(DelMsg *del_msg, int _day_id)
	{
		//如果有数据，则删除
		if (vms_ser.find(del_msg->id) != vms_ser.end())
		{
			vms_ser[del_msg->id].own_ser->removeVM(&vms_ser[del_msg->id], vms_ser[del_msg->id].a_b, _day_id);
			vms_ser.erase(del_msg->id);
			del_msg->dealed = true;
		}
	}

	//冒泡排序  按cpu与mem的和  由大到小排序
	void bubble(vector<VM2Server*> &vms)
	{
		int len = vms.size();
		for (int i = 0; i < len; i++) {//控制总的趟数
			for (int j = 1; j < len - i; ++j) {//一次冒泡排序的结果
				if ((vms[j - 1]->vm.cpu + vms[j - 1]->vm.memory) < (vms[j]->vm.cpu + vms[j]->vm.memory))
					swap(vms[j - 1], vms[j]);
			}
		}
	}

	//快速排序  按cpu与mem的和  由大到小排序
	int partition(vector<VM2Server*> &vms , int begin , int end)
	{
		int key = vms[end]->vm.cpu + vms[end]->vm.memory;
		int i = begin - 1;
		for (unsigned int j = begin; j < end;j++)
		{
			if((vms[j]->vm.cpu + vms[j]->vm.memory)>=key)
			{
				i++;
				swap(vms[i],vms[j]);
			}
		}
		swap(vms[i+1],vms[end]);
		return (i + 1);
	}
	void quickSort(vector<VM2Server*> &vms, int begin , int end)
	{
		int ret = 0;
		if(begin < end)
			{
				ret = partition(vms, begin, end);
				quickSort(vms,begin,ret-1);
				quickSort(vms,ret+1,end);
			}
	}

	//输出每日信息
	void coutDayMsg(int _day_id)
	{
		//购买型号的数量   bug  哈希表无序
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
		cout << "(migration, " << mig_msgs_day.size() << ")\n";
		for (int i = 0; i < mig_msgs_day.size(); i++)
		{
			cout << "(" << mig_msgs_day.at(i).vm_id << "," << mig_msgs_day.at(i).aim_id;
			if (mig_msgs_day.at(i).a_b == -1)
				cout << ")\n";
			else if (mig_msgs_day.at(i).a_b == 0)
				cout << ",A)\n";
			else if (mig_msgs_day.at(i).a_b == 1)
				cout << ",B)\n";
		}
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
		fflush(stdout);
	}

	void cout2File(ofstream &out_file, int _day_id)
	{
		//购买型号的数量
		if (own_sers.pur_sers.find(_day_id) != own_sers.pur_sers.end())  //如果当天购买不为空
		{
			out_file << "(purchase, " << own_sers.pur_sers.at(_day_id).size() << ")\n";
			for (auto it = own_sers.pur_sers.at(_day_id).begin(); it != own_sers.pur_sers.at(_day_id).end(); ++it)
			{
				//if (max_cpu_ser.cpu < _data_hand->servers.at(it->first).cpu)
				//型号及其对应购买数量
				out_file << "(" << own_sers.pur_sers.at(_day_id).at(it->first).type << ", " << own_sers.pur_sers.at(_day_id).at(it->first).num << ")\n";

			}
		}
		else
		{
			out_file << "(purchase, 0)\n";
		}
		//迁移数量
		out_file << "(migration, " << mig_msgs_day.size() << ")\n";
		for (int i = 0; i < mig_msgs_day.size(); i++)
		{
			out_file << "(" << mig_msgs_day.at(i).vm_id << ", " << mig_msgs_day.at(i).aim_id;
			if (mig_msgs_day.at(i).a_b == -1)
				out_file << ")\n";
			else if (mig_msgs_day.at(i).a_b == 0)
				out_file << ", A)\n";
			else if (mig_msgs_day.at(i).a_b == 1)
				out_file << ", B)\n";
		}

		//虚拟机部署到服务器 id和节点。
		for (int i = 0; i < data_hand->requests_all->at(_day_id).add_req.size(); i++)  //根据请求id 顺序输出
		{
			out_file << "(" << vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).own_ser->id;
			if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == -1)
				out_file << ")\n";
			else if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == 0)
				out_file << ", A)\n";
			else if (vms_ser.at(data_hand->requests_all->at(_day_id).add_req.at(i).id).a_b == 1)
				out_file << ", B)\n";
		}
	}

	void coutAllSersUsage()
	{
		for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		{
			iter->coutUsage();
		}
		coutAllCosts();
	}
	
	void coutMsg2CSV(ofstream &out_CSV ,int _day_id)
	{

		out_CSV << _day_id <<","<< (3 * vms_ser.size() / 100) <<","
			    << mig_num_day << endl;
		//输出每日请求的删除率
		// out_CSV << _day_id << "," << (float)(data_hand->requests_all->at(_day_id).del_req.size()) /(float)(data_hand->requests_all->at(_day_id).day_request.size()) << endl;
		// for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		// {
		// 	out_CSV <<  iter->usage_cpu << "," << iter->usage_mem << endl;
		// }
	}

	void coutAllCosts()
	{
		int all_costs = 0, hard_cost = 0;
		for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		{
			iter->calCosts(all_day_num);
			all_costs += iter->cost_all;
			hard_cost += iter->ser.hardware_cost;
		}
		cout << "costs:" << all_costs << "\n";
		cout << "hard_cost:" << hard_cost << "\n";
		cout << "all_day_use_cost:" << all_costs - hard_cost << "\n";

		cout << "all_sers_num :" << own_sers.using_ser.size() << "\n";
		cout << "all_vm_num :" << vms_ser.size()<< "\n";
	}

private:
	
	AllServers own_sers;  //拥有的所有服务器              
	unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器  <id,对应服务器>
	ServersData day_ser_select;    //当天选择的服务器类型
	int max_cpu, max_mem;  //每天请求中最大cpu与最大mem
	int all_day_num=0; //总天数
	vector<MigrationMsg> mig_msgs_day;  //每天的迁移
	int mig_num_day = 0; //迁移次数
};



#endif