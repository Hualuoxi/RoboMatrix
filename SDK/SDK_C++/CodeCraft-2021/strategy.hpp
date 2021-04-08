#ifndef STRATEGY
#define STRATEGY

#include "data_handling.hpp"
#include <list>
#include <cmath>
#include <time.h>

struct OwnServer;  //声明
// #define DEBUG
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
	float usage_cpu_A,usage_cpu_B; 
	float usage_mem_A,usage_mem_B; 
	float cost_all;   //总成本
	int use_days;    //使用总天数
	float balDeg_All;
	float balDeg_A;
	float balDeg_B;

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
		use_days = 0;

		balDeg_A  = cpu_A_left / memory_A_left;
		balDeg_B = cpu_B_left / memory_B_left;
		balDeg_All = ser.cpu / ser.memory;
		usage_cpu_A = usage_cpu_B = 0;
		usage_mem_A = usage_mem_B = 0;
	}
	void updateBalDeg()
	{
		if(memory_A_left == 0 && memory_B_left == 0)
		{
			balDeg_A = 1000;
			balDeg_B = 1000;
			balDeg_All = 1000;
		}
		else if(memory_A_left == 0)
		{
			balDeg_A = 1000;
			balDeg_B = cpu_B_left / memory_B_left;
			balDeg_All = (cpu_B_left + cpu_A_left) / (memory_B_left + memory_A_left);
		}
		else if(memory_B_left == 0)
		{
			balDeg_A = cpu_A_left / memory_A_left;
			balDeg_B = 1000;
			balDeg_All = (cpu_B_left + cpu_A_left) / (memory_B_left + memory_A_left);
		}
		else
		{
			balDeg_A = cpu_A_left / memory_A_left;
			balDeg_B = cpu_B_left / memory_B_left;
			balDeg_All = (cpu_B_left + cpu_A_left) / (memory_B_left + memory_A_left);
		}
		
	}

	//将虚拟机放入服务器  （-1 双节点    0 a  1 b）
	bool insertVM(VM2Server *_vm, int _day_id)
	{
		if (_vm->vm.node == 1)  //双节点 
		{
			_vm->a_b = -1;
			if (cpu_A_left >= _vm->vm.cpu / 2. && cpu_B_left >= _vm->vm.cpu / 2.  && memory_A_left >= _vm->vm.memory / 2. && memory_B_left >= _vm->vm.memory / 2.)
			{
				cpu_A_left -= _vm->vm.cpu / 2.;       usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);
				memory_A_left -= _vm->vm.memory / 2.; usage_mem_A =  1- memory_A_left/(ser.memory/2);
				cpu_B_left -= _vm->vm.cpu / 2.;		  usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);
				memory_B_left -= _vm->vm.memory / 2.; usage_mem_B =  1- memory_B_left/(ser.memory/2);
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

				vms.push_back(_vm);
				updateBalDeg();
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
				cpu_A_left -= _vm->vm.cpu;        usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);
				memory_A_left -= _vm->vm.memory;  usage_mem_A =  1- memory_A_left/(ser.memory/2);
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

				vms.push_back(_vm);
				updateBalDeg();
				return true;
			}
			else if (cpu_B_left >= _vm->vm.cpu && memory_B_left >= _vm->vm.memory)
			{
				_vm->a_b = 1;
				cpu_B_left -= _vm->vm.cpu;         usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);
				memory_B_left -= _vm->vm.memory;   usage_mem_B =  1- memory_B_left/(ser.memory/2);
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

				vms.push_back(_vm);
				updateBalDeg();
				return true;
			}
			else
				return false;
		}
	}
	bool insertVMByBD(VM2Server *_vm, int _day_id , float BD_coe)
	{
		updateBalDeg();
		if (_vm->vm.node == 1)  //双节点 
		{
			_vm->a_b = -1;
			if (cpu_A_left >= _vm->vm.cpu / 2. &&
			    cpu_B_left >= _vm->vm.cpu / 2. && 
				memory_A_left >= _vm->vm.memory / 2. && 
				memory_B_left >= _vm->vm.memory / 2. &&
				(fabs(balDeg_All - (_vm->vm.cpu / _vm->vm.memory)) <= BD_coe))
				{
					cpu_A_left -= _vm->vm.cpu / 2.;       usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);
					memory_A_left -= _vm->vm.memory / 2.; usage_mem_A =  1- memory_A_left/(ser.memory/2);
					cpu_B_left -= _vm->vm.cpu / 2.;		  usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);
					memory_B_left -= _vm->vm.memory / 2.; usage_mem_B =  1- memory_B_left/(ser.memory/2);
					usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
					usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

					vms.push_back(_vm);
					updateBalDeg();
					return true;	
				}
			else
				return false;
		}
		else  // 单节点
		{
			if (cpu_A_left >= _vm->vm.cpu  &&  memory_A_left >= _vm->vm.memory &&
				(fabs(balDeg_A - (_vm->vm.cpu / _vm->vm.memory)) <= BD_coe))
			{
				_vm->a_b = 0;
				cpu_A_left -= _vm->vm.cpu / 2.;       usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);
				memory_A_left -= _vm->vm.memory / 2.; usage_mem_A =  1- memory_A_left/(ser.memory/2);
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

				vms.push_back(_vm);
				updateBalDeg();
				return true;
			}
			else if (cpu_B_left >= _vm->vm.cpu && memory_B_left >= _vm->vm.memory &&
			 (fabs(balDeg_B - (_vm->vm.cpu / _vm->vm.memory)) <= BD_coe))
			{
				_vm->a_b = 1;
				cpu_B_left -= _vm->vm.cpu / 2.;		  usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);
				memory_B_left -= _vm->vm.memory / 2.; usage_mem_B =  1- memory_B_left/(ser.memory/2);
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;

				vms.push_back(_vm);
				updateBalDeg();
				return true;
			}
			else
				return false;
		}
	}
	//去掉虚拟机  
	void removeVM(VM2Server *_vm, int _a_b, int _day_id,bool _del_vm=true)
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
		if(_del_vm)
			vms.remove(_vm);
		usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);  usage_mem_A =  1- memory_A_left/(ser.memory/2);
		usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);  usage_mem_B =  1- memory_B_left/(ser.memory/2);
		usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
		usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
	}

	void removeVM(VM2Server *_vm)
	{
		vms.remove(_vm);
	}

	//将虚拟机放回服务器
	void putVMBack(VM2Server *_vm, int _a_b)
	{
		if (_a_b == -1)  //双节点 
		{
			cpu_A_left -= _vm->vm.cpu / 2.;
			memory_A_left -= _vm->vm.memory / 2.;
			cpu_B_left -= _vm->vm.cpu / 2.;
			memory_B_left -= _vm->vm.memory / 2.;
		}
		else if (_a_b == 0)
		{
			cpu_A_left -= _vm->vm.cpu;
			memory_A_left -= _vm->vm.memory;
		}
		else if (_a_b == 1)
		{
			cpu_B_left -= _vm->vm.cpu;
			memory_B_left -= _vm->vm.memory;
		}
		usage_cpu_A = 1- cpu_A_left/(ser.cpu/2);  usage_mem_A =  1- memory_A_left/(ser.memory/2);
		usage_cpu_B = 1- cpu_B_left/(ser.cpu/2);  usage_mem_B =  1- memory_B_left/(ser.memory/2);
		usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
		usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
	}

	//删除所有虚拟机  
	void removeAllVM(int _day_id)
	{
		cpu_A_left = ser.cpu / 2.;
		memory_A_left = ser.memory / 2.;
		cpu_B_left = ser.cpu / 2.;
		memory_B_left = ser.memory / 2.;
		usage_cpu = usage_mem = 0;
		use_days = 0;
		vms.clear();
	}

	//打印利用率
	void coutUsage()
	{
		cout << "CPU: " << usage_cpu << "  mem: " << usage_mem << "\n";
	}
	//计算总成本 
	void calCosts(int _day_id)
	{
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
	return  (p1.ser.energy_day/(p1.ser.cpu+p1.ser.memory) < p2.ser.energy_day/(p2.ser.cpu+p2.ser.memory));		//按二者和排序
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
	//迁移用服务器
	OwnServer *mig_ser;  //id = 0
	//<天数,<型号 数量> >  天数从0开始
	unordered_map<int, unordered_map<string, PurSerData>> pur_sers;

	unsigned int num = 1;   //计数  每次添加服务器就加1 

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
	void sortbyEnergy()
	{
		 using_ser.sort(myCompareByenergy);
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
		if(all_day_num==0) 
		{
			all_day_num = data_hand->requests_all->size() - 1;  //从0开始 
			findMaxCostSer();
		}
		mig_msgs_day.clear();  //清除上一天数据
		mig_num_day = migByUsageNumDay =  migByEnergyDay =  migByCpuMemDay = 0;
		// float del_rat = (float)(data_hand->requests_all->at(_day_id).del_req.size()) /(float)(data_hand->requests_all->at(_day_id).day_request.size());
		migration(&mig_msgs_day, _day_id);
		migLowCPU2Mem(&mig_msgs_day, _day_id);

		
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
		unordered_map<int, DelMsg> del_msg_day;   //每日的删除请求
		DelMsg msg;
		for (int i = 0; i < dat_req->del_req.size(); i++)
		{
			msg.id = dat_req->del_req.at(i).id;
			msg.dealed = false;
			del_msg_day[msg.id] = msg;
		}
		//排序
		quickSort(vms_node_d,0,vms_node_d.size()-1);
		quickSort(vms_node_s,0,vms_node_s.size()-1);
		//先将虚拟机放入已有服务器  先双后单
		// own_sers.sortbyEnergy();
		for (int j = 0; j < vms_node_d.size(); j++)
		{
			addVm2Ser(vms_node_d.at(j), false, _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			addVm2Ser(vms_node_s.at(j), false, _day_id);
		}
		selectSer(dat_req, _day_id,1.1f);

		//将剩下的虚拟机放入服务器  先双后单
		// own_sers.sortbyEnergy();
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

		// if(mig_num_day == (3 * vms_ser.size() / 100))
		// 	migMax =true;
		// else
		// 	migMax =false;
		//服务器排序
		own_sers.sort();
		// if(own_sers.using_ser.end()->usage_cpu<=0.5&&own_sers.using_ser.end()->usage_mem<=0.5) return;
		// else if((_day_id+1) <= data_hand->day_num)
		// {
		// 	del_rat = (float)(data_hand->requests_all->at(_day_id+1).del_req.size()) /(float)(data_hand->requests_all->at(_day_id+1).day_request.size());
		// 	if(del_rat <= 0.4)
		// 		own_sers.addSer(day_ser_select, _day_id);  //添加服务器用于转移
		// }
		claSersUseday();
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
			if (it->usage_cpu < 0.75 || it->usage_mem < 0.75) //利用率低  0.8 timeout
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
							// inset_success = it2->insertVMByBD(*it_vm, _day_id,0.2);
							inset_success = it2->insertVM(*it_vm, _day_id);
							if (inset_success)
							{
								MigrationMsg mig;
								mig.vm_id = (*it_vm)->vm_id;
								mig.aim_id = it2->id;
								mig.a_b = (*it_vm)->a_b;
								mig_msg->push_back(mig);
								migByUsageNum++;
								(*it_vm)->own_ser = &*it2;
								//循环的时候不能删除元素  记录下来
								vms_remove.push_back(*it_vm);
								mig_num_day++;
								migByUsageNumDay++;
								break;
							}
						}
					}
					// if(!inset_success)
					// {
					// 	for (auto it2 = own_sers.using_ser.begin(); it2 != own_sers.using_ser.end(); ++it2)
					// 	{
					// 		if (mig_sers.find(it2->id) == mig_sers.end())   //不在迁移表中
					// 		{
					// 			if (mig_num_day >= mig_num)
					// 				break;
					// 			(*it_vm)->a_b_mig = (*it_vm)->a_b;
					// 			inset_success = it2->insertVM(*it_vm, _day_id);
					// 			if (inset_success)
					// 			{
					// 				MigrationMsg mig;
					// 				mig.vm_id = (*it_vm)->vm_id;
					// 				mig.aim_id = it2->id;
					// 				mig.a_b = (*it_vm)->a_b;
					// 				mig_msg->push_back(mig);
					// 				migByUsageNum++;
					// 				(*it_vm)->own_ser = &*it2;
					// 				//循环的时候不能删除元素  记录下来
					// 				vms_remove.push_back(*it_vm);
					// 				mig_num_day++;
					// 				migByUsageNumDay++;
					// 				break;
					// 			}
					// 		}
					// 	}
					// }
				}
				for (int i = 0; i < vms_remove.size(); i++)
				{
					it->removeVM(vms_remove.at(i), vms_remove.at(i)->a_b_mig, _day_id);
				}
			}
		}
		//cout << "mig_num: "<< mig_num << "  "<< vms_ser.size() <<"  "<< vms_ser.size()*5 /1000 <<"\n";
	}

	void migLowCPU2Mem(vector<MigrationMsg> *mig_msg, int _day_id )
	{
		vector<OwnServer*> sers_low_cpu, sers_low_mem;  //
		int mig_num = 3 * vms_ser.size() / 100;
		// cout <<"mig_num :" <<mig_num <<"\n";
		// cout <<"	mig_1: " << mig_num_day <<"\n";
		float usage_val= 0.15;  
		for (auto it1 = own_sers.using_ser.begin(); it1 != own_sers.using_ser.end(); ++it1)
		{
			if (it1->usage_mem_A  - it1->usage_cpu_A > usage_val  || it1->usage_mem_B  - it1->usage_cpu_B > usage_val)
			{
				sers_low_cpu.push_back(&*it1);
			}
			if (it1->usage_cpu_A - it1->usage_mem_A > usage_val  || it1->usage_cpu_B - it1->usage_mem_B > usage_val)
			{
				sers_low_mem.push_back(&*it1);
			}
		}
		if(sers_low_cpu.size()==0  ||  sers_low_mem.size()==0) return;
		usage_val = 0.05;
		for (unsigned int i = 0; i < sers_low_cpu.size(); i++)
		{
			if (mig_num_day >= mig_num)
				break;
			vector<VM2Server*> vms_remove;
			bool balance = false;
			for (auto it_vm = sers_low_cpu.at(i)->vms.begin(); it_vm != sers_low_cpu.at(i)->vms.end(); ++it_vm)
			{
				if (mig_num_day >= mig_num)
					break;
				if( sers_low_cpu.at(i)->usage_mem - sers_low_cpu.at(i)->usage_cpu < usage_val)
					break;
				if((*it_vm)->vm.memory / (float)(*it_vm)->vm.cpu < 1.3) 
					continue;
				bool inset_success = false;
				bool inset_success2 = false;
				vector<VM2Server*> vms_mem_remove;
				for (unsigned int mem_j = 0; mem_j < sers_low_mem.size(); mem_j++)
				{
					if (mig_num_day+3 >= mig_num)
						break;
					if(sers_low_mem.at(mem_j)->usage_cpu - sers_low_mem.at(mem_j)->usage_mem < usage_val)
						continue;
					for (auto it_vm_mem = sers_low_mem.at(mem_j)->vms.begin(); it_vm_mem != sers_low_mem.at(mem_j)->vms.end(); ++it_vm_mem)
					{
						// 在服务器中先删除一个虚拟机，判断其能否放下该虚拟机
						sers_low_cpu.at(i)->removeVM(*it_vm,(*it_vm)->a_b,_day_id,false);
						sers_low_mem.at(mem_j)->removeVM(*it_vm_mem,(*it_vm_mem)->a_b,_day_id,false);
						(*it_vm)->a_b_mig = (*it_vm)->a_b;
						(*it_vm_mem)->a_b_mig = (*it_vm_mem)->a_b;
						inset_success = sers_low_cpu.at(i)->insertVM(*it_vm_mem, _day_id);
						inset_success2 = sers_low_mem.at(mem_j)->insertVM(*it_vm, _day_id);

						if(inset_success && inset_success2)
						{
							
							addMigMsg(mig_msg,(*it_vm_mem),own_sers.mig_ser);
							
							addMigMsg(mig_msg,(*it_vm), sers_low_mem.at(mem_j));
							
							addMigMsg(mig_msg,(*it_vm_mem), sers_low_cpu.at(i));
							
							//循环的时候不能删除元素  记录下来
							vms_remove.push_back(*it_vm);
							vms_mem_remove.push_back(*it_vm_mem);
							mig_num_day+=3;
							
							break;
						}
						else
						{
							sers_low_cpu.at(i)->putVMBack(*it_vm,(*it_vm)->a_b_mig);
							sers_low_mem.at(mem_j)->putVMBack(*it_vm_mem,(*it_vm_mem)->a_b_mig);
							if(inset_success) { //还原
								sers_low_cpu.at(i)->removeVM(*it_vm_mem,(*it_vm_mem)->a_b,_day_id);
								(*it_vm_mem)->a_b = (*it_vm_mem)->a_b_mig;
							}
							if(inset_success2) { //还原
								sers_low_mem.at(mem_j)->removeVM(*it_vm,(*it_vm)->a_b,_day_id);
								(*it_vm)->a_b = (*it_vm)->a_b_mig;
							}
						}
					}
					for (int s = 0; s < vms_mem_remove.size(); s++)
					{
						sers_low_mem.at(mem_j)->removeVM(vms_mem_remove.at(s));
					}
					if(inset_success && inset_success2)
						break;
				}
			}
			
			for (int s = 0; s < vms_remove.size(); s++)
			{
				sers_low_cpu.at(i)->removeVM(vms_remove.at(s));
			}
		}
		// cout <<"	mig_2: " << mig_num_day <<"\n";

		for (unsigned int i = 0; i < sers_low_mem.size(); i++)
		{
			if (mig_num_day >= mig_num)
				break;
			vector<VM2Server*> vms_remove;
			bool balance = false;
			for (auto it_vm = sers_low_mem.at(i)->vms.begin(); it_vm != sers_low_mem.at(i)->vms.end(); ++it_vm)
			{
				if (mig_num_day >= mig_num)
					break;
				if( sers_low_mem.at(i)->usage_cpu - sers_low_mem.at(i)->usage_mem < usage_val)
					break;
				if((*it_vm)->vm.cpu / (float)(*it_vm)->vm.memory < 1.5) 
					continue;
				bool inset_success = false;
				bool inset_success2 = false;
				vector<VM2Server*> vms_cpu_remove;
				for (unsigned int cpu_j = 0; cpu_j < sers_low_cpu.size(); cpu_j++)
				{
					if (mig_num_day+3 >= mig_num)
						break;
					if(sers_low_cpu.at(cpu_j)->usage_mem - sers_low_cpu.at(cpu_j)->usage_cpu < usage_val)
						continue;
					for (auto it_vm_cpu = sers_low_cpu.at(cpu_j)->vms.begin(); it_vm_cpu != sers_low_cpu.at(cpu_j)->vms.end(); ++it_vm_cpu)
					{
						// 在服务器中先删除一个虚拟机，判断其能否放下该虚拟机
						sers_low_mem.at(i)->removeVM(*it_vm,(*it_vm)->a_b,_day_id,false);
						sers_low_cpu.at(cpu_j)->removeVM(*it_vm_cpu,(*it_vm_cpu)->a_b,_day_id,false);
						(*it_vm)->a_b_mig = (*it_vm)->a_b;
						(*it_vm_cpu)->a_b_mig = (*it_vm_cpu)->a_b;
						inset_success = sers_low_mem.at(i)->insertVM(*it_vm_cpu, _day_id);
						inset_success2 = sers_low_cpu.at(cpu_j)->insertVM(*it_vm, _day_id);

						if(inset_success && inset_success2)
						{
							
							addMigMsg(mig_msg,(*it_vm_cpu),own_sers.mig_ser);
							
							addMigMsg(mig_msg,(*it_vm), sers_low_cpu.at(cpu_j));
							
							addMigMsg(mig_msg,(*it_vm_cpu), sers_low_mem.at(i));
							
							//循环的时候不能删除元素  记录下来
							vms_remove.push_back(*it_vm);
							vms_cpu_remove.push_back(*it_vm_cpu);
							mig_num_day+=3;
							break;
						}
						else
						{
							sers_low_mem.at(i)->putVMBack(*it_vm,(*it_vm)->a_b_mig);
							sers_low_cpu.at(cpu_j)->putVMBack(*it_vm_cpu,(*it_vm_cpu)->a_b_mig);
							if(inset_success) { //还原
								sers_low_mem.at(i)->removeVM(*it_vm_cpu,(*it_vm_cpu)->a_b,_day_id);
								(*it_vm_cpu)->a_b = (*it_vm_cpu)->a_b_mig;
							}
							if(inset_success2) { //还原
								sers_low_cpu.at(cpu_j)->removeVM(*it_vm,(*it_vm)->a_b,_day_id);
								(*it_vm)->a_b = (*it_vm)->a_b_mig;
							}
						}
					}
					for (int s = 0; s < vms_cpu_remove.size(); s++)
					{
						sers_low_cpu.at(cpu_j)->removeVM(vms_cpu_remove.at(s));
					}
					if(inset_success && inset_success2)
						break;
				}
			}
			
			for (int s = 0; s < vms_remove.size(); s++)
			{
				sers_low_mem.at(i)->removeVM(vms_remove.at(s));
			}
		}
		// cout <<"	mig_3: " << mig_num_day <<"\n";
	}


	void migLowCPU2MemByMidSer(vector<MigrationMsg> *mig_msg, int _day_id , float rat)
	{
		vector<OwnServer*> sers_low_cpu, sers_low_mem;  //
		OwnServer* migMidSer = &*own_sers.using_ser.end();
		int mig_num = 3 * vms_ser.size() / 100;
		for (auto it1 = own_sers.using_ser.begin(); it1 != own_sers.using_ser.end(); ++it1)
		{
			if ((1- it1->balDeg_All) > rat ||(1- it1->balDeg_A)  > rat ||(1- it1->balDeg_B) > rat )
				sers_low_cpu.push_back(&*it1);
			if ((it1->balDeg_All -1)  > rat ||(it1->balDeg_A-1)   > rat ||(it1->balDeg_B-1) > rat )
				sers_low_mem.push_back(&*it1);
		}
		if (sers_low_cpu.size() == 0 || sers_low_mem.size() == 0)  return;
		for (unsigned int i = 0; i < sers_low_cpu.size(); i++)
		{
			if (mig_num_day >= mig_num)
				break;
			vector<VM2Server*> vms_remove;
			//找到该服务器中平衡度偏移最大的虚拟机
			float maxBal =0.0;
			int maxVmNode = 0;
			VM2Server* maxVm =nullptr;
			for (auto it_vm = sers_low_cpu.at(i)->vms.begin(); it_vm != sers_low_cpu.at(i)->vms.end(); ++it_vm)
			{
				if(((*it_vm)->a_b == -1) &&
				   (maxBal < (fabs)(((*it_vm)->vm.cpu/(*it_vm)->vm.memory) - (sers_low_cpu.at(i)->ser.cpu/sers_low_cpu.at(i)->ser.memory)))) //再双节点上
				{
						maxBal = (fabs)((*it_vm)->vm.cpu/(*it_vm)->vm.memory - sers_low_cpu.at(i)->balDeg_All);
						maxVm = *it_vm;
						maxVmNode = -1;
				}
				else if(((*it_vm)->a_b == 0) &&
				   (maxBal < (fabs)((*it_vm)->vm.cpu/(*it_vm)->vm.memory - sers_low_cpu.at(i)->balDeg_A))) //A节点
				{
					maxBal = (fabs)((*it_vm)->vm.cpu/(*it_vm)->vm.memory - sers_low_cpu.at(i)->balDeg_A);
					maxVm = *it_vm;
					maxVmNode = 0;
				}
				else if(((*it_vm)->a_b == 1) &&
				   (maxBal < (fabs)((*it_vm)->vm.cpu/(*it_vm)->vm.memory - sers_low_cpu.at(i)->balDeg_B))) //B节点
				{
					maxBal = (fabs)((*it_vm)->vm.cpu/(*it_vm)->vm.memory - sers_low_cpu.at(i)->balDeg_B);
					maxVm = *it_vm;
					maxVmNode = 1;
				}
			}

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
						migByCpuMem++;

						(*it_vm)->own_ser = sers_low_mem.at(j);
						//循环的时候不能删除元素  记录下来
						vms_remove.push_back(*it_vm);
						mig_num_day++;
						migByCpuMemDay++;
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
					inset_success = sers_low_cpu.at(j)->insertVMByBD(*it_vm, _day_id,0.3);
					if (inset_success)
					{
						MigrationMsg mig;
						mig.vm_id = (*it_vm)->vm_id;
						mig.aim_id = sers_low_cpu.at(j)->id;
						mig.a_b = (*it_vm)->a_b;
						mig_msg->push_back(mig);
						migByCpuMem++;

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

	void migByReInsert(vector<MigrationMsg> *mig_msg, int _day_id,float rat)
	{
		vector<OwnServer*> sers_ReInster;  //
		vector<VM2Server*> vms_ReInster;
		int mig_num = 3 * vms_ser.size() / 100;
		int mig_tep = mig_num_day;
		//找到需要重新摆放的服务器
		for (auto it1 = own_sers.using_ser.begin(); it1 != own_sers.using_ser.end(); ++it1)
		{
			if (fabs(it1->usage_mem - it1->usage_cpu) > rat)
			{
				if(mig_tep >= it1->vms.size())
				{
					sers_ReInster.push_back(&*it1);
					mig_tep-=it1->vms.size();
				}
			}
		}
		if(sers_ReInster.size() == 0) return ;
		//找到需要重新摆放的服务器中不合适的虚拟机
		for(int i =0 ;i <sers_ReInster.size();i++)
		{
			// float CMRat = sers_ReInster.at(i)->ser.cpu / sers_ReInster.at(i)->ser.memory;
			for(auto it_vm = sers_ReInster.at(i)->vms.begin(); it_vm != sers_ReInster.at(i)->vms.end(); ++it_vm)
			{
				vms_ReInster.push_back(*it_vm);
			}
			sers_ReInster.at(i)->removeAllVM(_day_id);
		}
		//
		quickSort(vms_ReInster,0,vms_ReInster.size()-1);
		quickSort(sers_ReInster,0 , sers_ReInster.size()-1);
		bool inset_success = false;
		// int allVmsNum = vms_ReInster.size();
		for(int i = 0;i<vms_ReInster.size();i++)
		{
			for (unsigned int j = 0; j < sers_ReInster.size(); j++)
			{
				if (mig_num_day >= mig_num)
					break;
				vms_ReInster.at(i)->a_b_mig = vms_ReInster.at(i)->a_b;
				inset_success = sers_ReInster.at(j)->insertVMByBD(vms_ReInster.at(i), _day_id,0.5);
				if (inset_success)
				{
					//如果之前不在这个服务器 就 不迁移  放回去
					if((sers_ReInster.at(j)->id != vms_ReInster.at(i)->own_ser->id) && (vms_ReInster.at(i)->a_b_mig == vms_ReInster.at(i)->a_b))
					{
						MigrationMsg mig;
						mig.vm_id = vms_ReInster.at(i)->vm_id;
						mig.aim_id = sers_ReInster.at(j)->id;
						mig.a_b = vms_ReInster.at(i)->a_b;
						mig_msg->push_back(mig);
						migByCpuMem++;
						vms_ReInster.at(i)->own_ser = sers_ReInster.at(j);
						mig_num_day++;
					}
					break;
				}
					
			}
			if(!inset_success)
				cout << "migByReInsert Error!" <<endl;
		}
		// if(vms_ReInster.size() != 0 )
			
	}


	void migrationByEnergy(vector<MigrationMsg> *mig_msg, int _day_id)
	{
		unordered_map<int, OwnServer*> mig_sers; //迁移的服务器或者空闲的 之后就不往里面迁移了
		int mig_num = 3 * vms_ser.size() / 100;
		if (own_sers.using_ser.size() == 0) return;

		for (auto it = --own_sers.using_ser.end(); it != own_sers.using_ser.begin(); it--)
		{
			if (mig_num_day >= mig_num)
				break;
			if (it->usage_cpu==0 ||  it->usage_mem == 0)
			{
				mig_sers[it->id] = &*it;
				continue; //利用率为0就不考虑了
			}
			mig_sers[it->id] = &*it;
			vector<VM2Server*> vms_remove;
			int serMigNum =0;
			for (auto it_vm = it->vms.begin(); it_vm != it->vms.end(); ++it_vm)
			{
				if (mig_num_day >= mig_num)
					break;
				// if( serMigNum / (float)(it->vms.size()) > 0.5f )	
				// 	break;
				bool inset_success = false;
				for (auto it2 = own_sers.using_ser.begin(); it2 != it; ++it2)
				{
					if (mig_sers.find(it2->id) == mig_sers.end())   //不在迁移表中
					{
						// if(it2->usage_cpu + it2->usage_mem > 1.8)
						// 	continue;
						if (mig_num_day >= mig_num)
							break;
						(*it_vm)->a_b_mig = (*it_vm)->a_b;
						inset_success = it2->insertVM(*it_vm, _day_id);
						if (inset_success)
						{
							serMigNum++;
							MigrationMsg mig;
							mig.vm_id = (*it_vm)->vm_id;
							mig.aim_id = it2->id;
							mig.a_b = (*it_vm)->a_b;
							migByEnergy++;
							mig_msg->push_back(mig);

							(*it_vm)->own_ser = &*it2;
							//循环的时候不能删除元素  记录下来
							vms_remove.push_back(*it_vm);
							mig_num_day++;
							migByEnergyDay++;
							break;
						}
					}
				}	
				// if(!inset_success)
				// 	break;
			}
			for (int i = 0; i < vms_remove.size(); i++)
			{
				it->removeVM(vms_remove.at(i), vms_remove.at(i)->a_b_mig, _day_id);
			}

		}
		//cout << "mig_num: "<< mig_num << "  "<< vms_ser.size() <<"  "<< vms_ser.size()*5 /1000 <<"\n";
	}
#if 0
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
#endif
	//根据每天没有处理过的虚拟机数据总和 选择服务器
	void selectSer(DayRequestData *dat_req, int _day_id,float rat)
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
			int c_num = ceil(cpu_sum / it->second.cpu);  //向上取整
			int m_num = ceil(mem_sum / it->second.memory);
			int _num = (c_num > m_num) ? c_num : m_num; 
			_num = ceil(_num / rat);
			//数量 * 硬件成本加剩余天数的使用成本
			int it_cost = _num * (it->second.hardware_cost  +  (all_day_num - _day_id) * it->second.energy_day);
			// int it_cost = _num * it->second.hardware_cost;
			//申请的型号需要满足cpu与mem都大于当前申请的最大值

				if (cost == 0)  //未初始化
				{
					cost = it_cost;
					_type = it->second.server_type;
					num = _num;
				}
				else if (cost > it_cost)
				{
					if (it->second.cpu >= max_cpu && it->second.memory >= max_mem)
					{
						cost = it_cost;
						_type = it->second.server_type;
						num = _num;
					}
				}
			
		}
		day_ser_select = data_hand->servers.at(_type);
	}

		//根据当前虚拟机选择性价比最高的服务器
	void selectSerByVM(VM2Server _vm2ser, int _day_id)
	{
		float costPerformance =0;
		float it_costPerformance =0;
		//根据每日性价比选择服务器
		for (auto it = data_hand->servers.begin(); it != data_hand->servers.end(); ++it)
		{
			//数量 * 硬件成本加剩余天数的使用成本
			it_costPerformance =  (float)(it->second.hardware_cost  +  (all_day_num - _day_id) * it->second.energy_day) /(float)(it->second.cpu + it->second.memory);
			if (costPerformance == 0)  //未初始化
			{
				costPerformance = it_costPerformance;
				day_ser_select = it->second;
			}
			else if (costPerformance > it_costPerformance)
			{
				if(_vm2ser.vm.node == 0) //单节点 按两倍算
				{
					if ((it->second.cpu >= (_vm2ser.vm.cpu *2)) && (it->second.memory >= (_vm2ser.vm.memory *2)))
					{
						costPerformance = it_costPerformance;
						day_ser_select = it->second;
						// _type = it->second.server_type;
					}
				}
				else
				{
					if ((it->second.cpu >= _vm2ser.vm.cpu) && (it->second.memory >= _vm2ser.vm.memory))
					{
						costPerformance = it_costPerformance;
						day_ser_select = it->second;
						// _type = it->second.server_type;
					}
				}
			}
			
		}
	}

	//添加虚拟机到服务器 add_new_ser:如果没有空间，是否申请新服务器
	void addVm2Ser(VM2Server *_vm2ser, bool add_new_ser, int _day_id)
	{
		bool inset_success = false;

		//判断使用中服务器中是否有空闲位置
		for (auto it = own_sers.using_ser.begin(); it != own_sers.using_ser.end(); it++)
		{
			//if(add_new_ser)
				inset_success = it->insertVM(_vm2ser, _day_id);
			// else
			// 	inset_success = it->insertVMByBD(_vm2ser, _day_id,0.2);
			if (inset_success)
			{
				_vm2ser->own_ser = &*it;
				_vm2ser->dealed = true;
				break;
			}
		}
		// if(!inset_success)
		// {
		// 	for (auto it = own_sers.using_ser.begin(); it != own_sers.using_ser.end(); it++)
		// 	{
		// 		if(add_new_ser)
		// 			inset_success = it->insertVM(_vm2ser, _day_id);
		// 		else
		// 			break;
		// 		if (inset_success)
		// 		{
		// 			_vm2ser->own_ser = &*it;
		// 			_vm2ser->dealed = true;
		// 			break;
		// 		}
		// 	}
		// }
		if (!add_new_ser) return;
		if (!inset_success)  //没有成功加入虚拟机  空间不足  申请新的服务器
		{
			bool inset_success2 = false;
			// selectSerByVM(*_vm2ser,_day_id);
			own_sers.addSer(day_ser_select, _day_id);  //添加服务器
			inset_success2 = own_sers.using_ser.back().insertVM(_vm2ser, _day_id);
			if (inset_success2)
			{
				_vm2ser->own_ser = &own_sers.using_ser.back();
				_vm2ser->dealed = true;
			}
			else
			{
				cout << " insertVM error! " << endl;
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

	void addMigMsg(vector<MigrationMsg> *mig_msg,VM2Server* vm,OwnServer* aim_ser)
	{
		MigrationMsg mig;
		mig.vm_id = vm->vm_id;
		mig.aim_id = aim_ser->id;
		mig.a_b = vm->a_b;
		mig_msg->push_back(mig);
		vm->own_ser = aim_ser;
	}

	void claSersUseday()
	{
		for (auto it2 = own_sers.using_ser.begin(); it2 != own_sers.using_ser.end(); ++it2)
		{
			if(it2->usage_cpu!=0 || it2->usage_mem!=0 )
				it2->use_days++;
		}
	}

	void findMaxCostSer()
	{
		ServersData max_cost_ser;
		for (auto it = data_hand->servers.begin(); it != data_hand->servers.end(); ++it)
		{
			if(max_cost_ser.hardware_cost ==0) //初值
				max_cost_ser = data_hand->servers.at(it->first);
			if (max_cost_ser.hardware_cost < data_hand->servers.at(it->first).hardware_cost)
				max_cost_ser = data_hand->servers.at(it->first);
		}
		own_sers.mig_ser = new OwnServer(0,max_cost_ser);
		//cout <<"max_cost_ser.server_type: " <<max_cost_ser.server_type <<"\n";
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
		int key = vms[end]->vm.cpu  + vms[end]->vm.memory ;
		int i = begin - 1;
		for (unsigned int j = begin; j < end;j++)
		{
			if((vms[j]->vm.cpu+ vms[j]->vm.memory)>=key)
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
	int partition(vector<OwnServer*> &sers , int begin , int end)
	{
		int key = sers[end]->ser.cpu + sers[end]->ser.memory;
		int i = begin - 1;
		for (unsigned int j = begin; j < end;j++)
		{
			if((sers[j]->ser.cpu + sers[j]->ser.memory)>=key)
			{
				i++;
				swap(sers[i],sers[j]);
			}
		}
		swap(sers[i+1],sers[end]);
		return (i + 1);
	}
	void quickSort(vector<OwnServer*> &sers, int begin , int end)
	{
		int ret = 0;
		if(begin < end)
			{
				ret = partition(sers, begin, end);
				quickSort(sers,begin,ret-1);
				quickSort(sers,ret+1,end);
			}
	}


	//输出每日信息
	void coutDayMsg(int _day_id)
	{
		if(_day_id == 0 )
		{
			cout << "(purchase, " << own_sers.pur_sers.at(_day_id).size()+1 << ")\n";
			cout << "(" << own_sers.mig_ser->ser.server_type << ", " << 1 << ")\n";
			for (auto it = own_sers.pur_sers.at(_day_id).begin(); it != own_sers.pur_sers.at(_day_id).end(); ++it)
			{
				//if (max_cpu_ser.cpu < _data_hand->servers.at(it->first).cpu)
				//型号及其对应购买数量
				cout << "(" << own_sers.pur_sers.at(_day_id).at(it->first).type << ", " << own_sers.pur_sers.at(_day_id).at(it->first).num << ")\n";
			}
		}
		else{
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
		int unUsedSers =0;
		for(auto it = own_sers.using_ser.begin() ; it != own_sers.using_ser.end();++it)
		{
			if (it->usage_cpu ==0 ||it->usage_mem ==0)
			{
				unUsedSers++;
			}
		}

		out_CSV << _day_id <<","<< (3 * vms_ser.size() / 100) <<","
			    << mig_num_day<<"," << migByUsageNumDay<<"," << migByCpuMemDay<<"," << migByEnergyDay<<","
				<<(float)(data_hand->requests_all->at(_day_id).del_req.size()) /(float)(data_hand->requests_all->at(_day_id).day_request.size())*4000<<","
				<< own_sers.using_ser.size() << "," << unUsedSers <<","
				<< endl;
		//输出每日请求的删除率
		// out_CSV << _day_id << "," << (float)(data_hand->requests_all->at(_day_id).del_req.size()) /(float)(data_hand->requests_all->at(_day_id).day_request.size()) << endl;
	}
	void coutDayUsage2CSV(ofstream &out_CSV ,int _day_id)
	{
		for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		{
			out_CSV << _day_id << ","<<  iter->usage_cpu << "," << iter->usage_mem << "," 
			<< (fabs)(iter->usage_mem - iter->usage_cpu) << endl;
		}
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
		cout << "migByUsageNum:" << migByUsageNum << "\n";
		cout << "migByEnergy:" << migByEnergy << "\n";
		cout << "migByCpuMem:" << migByCpuMem << "\n";
		cout << "allMigNum:" << migByUsageNum + migByEnergy + migByCpuMem << "\n";
	}


private:
	
	AllServers own_sers;  //拥有的所有服务器              
	unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器  <id,对应服务器>
	ServersData day_ser_select;    //当天选择的服务器类型
	int max_cpu, max_mem;  //每天请求中最大cpu与最大mem
	int all_day_num=0; //总天数
	vector<MigrationMsg> mig_msgs_day;  //每天的迁移
	int mig_num_day = 0,migByUsageNumDay = 0, migByEnergyDay = 0, migByCpuMemDay=0; //迁移次数
	int  migByUsageNum = 0, migByEnergy = 0, migByCpuMem=0;
	bool migMax =false;
};



#endif