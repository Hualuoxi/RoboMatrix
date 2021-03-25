#ifndef STRATEGY
#define STRATEGY

#include "data_handling.hpp"
#include <list>
#include "math.h"
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
		usage_cpu = usage_mem =0;
		day_start = use_days = 0;
		unused= false;
	}

	//将虚拟机放入服务器  （-1 双节点    0 a  1 b）
	bool insertVM(VM2Server *_vm  ,int _day_id)
	{
		if (_vm->vm.node == 1)  //双节点 
		{
			_vm->a_b = -1;
			if (cpu_A_left >= _vm->vm.cpu / 2. && cpu_B_left >= _vm->vm.cpu / 2.  && memory_A_left >= _vm->vm.memory/2. && memory_B_left>= _vm->vm.memory/2.)
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
				cpu_A_left -= _vm->vm.cpu ;
				memory_A_left -= _vm->vm.memory;
				usage_cpu = 1 - (cpu_A_left + cpu_B_left) / ser.cpu;
				usage_mem = 1 - (memory_A_left + memory_B_left) / ser.memory;
				if (day_start == 0)  day_start = _day_id; //记录起始天
				unused = false;
				vms.push_back(_vm);
				return true;
			}
			else if(cpu_B_left >= _vm->vm.cpu && memory_B_left >= _vm->vm.memory)
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
	void removeVM(VM2Server *_vm,int _a_b ,int _day_id)
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
			use_days += (_day_id - day_start+1);  //已经运行总天数
			day_start = 0;
			unused = true;
		}
	}

	//打印利用率
	void coutUsage()
	{
		cout<< "CPU: " << usage_cpu << "  mem: " << usage_mem  <<"\n";
	}
	//计算总成本 
	void calCosts(int _day_id)
	{
		if(unused == false)
			use_days += (_day_id - day_start+1);
		
		cost_all =  ser.hardware_cost + ser.energy_day * use_days ;
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
	return (p1.usage_cpu) > (p2.usage_cpu );		//按cpu排序
}

//拥有的所有服务器
struct AllServers
{
	//正在使用的服务器  只能使用list
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
	//排序
	void sort()
	{
		using_ser.sort(myCompare);
	}
	
};


class Strategy
{
public:
	Strategy(DataHandling *_data_hand) 
	{ 
		data_hand = _data_hand; 
		max_cpu_ser.cpu = max_mem_ser.memory = 0;
		all_day_num = _data_hand->requests_all->size()-1;  //从0开始
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

	void dealDayReq(DayRequestData *dat_req, int _day_id)
	{
		mig_msgs_day.clear();  //清除上一天数据
		max_cpu = max_mem = 0;
		//先将请求数据转换  转换为VM2Server格式
		for (int i = 0; i < dat_req->add_req.size(); i++)
		{
			vms_ser[dat_req->add_req.at(i).id].vm_id =dat_req->add_req.at(i).id;
			vms_ser[dat_req->add_req.at(i).id].dealed = false;
			vms_ser[dat_req->add_req.at(i).id].matched = false;
			vms_ser[dat_req->add_req.at(i).id].vm = data_hand->vms.at(dat_req->add_req.at(i).vm_type);
		}
		//删除的消息转化为 DelMsg 格式
		unordered_map<int,DelMsg> del_msg_day;   //每日的删除请求
		for (int i = 0; i < dat_req->del_req.size(); i++)
		{
			DelMsg msg;
			msg.id = dat_req->del_req.at(i).id;
			msg.dealed = false;
			del_msg_day[msg.id] = msg;
		}

		migration(&mig_msgs_day, _day_id);

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
		
		selectSer(dat_req, _day_id);

		/*vector<VM2Server*> vms_node_undealed;
		for (int i = 0; i < vms_node_d.size(); i++)
		{
			if (vms_node_d.at(i)->dealed == false)
				vms_node_undealed.push_back(vms_node_d.at(i));
		}
		for (int i = 0; i < vms_node_s.size(); i++)
		{
			if (vms_node_s.at(i)->dealed == false)
				vms_node_undealed.push_back(vms_node_s.at(i));
		}
		if(vms_node_undealed.size()>5)
			simulatedFire(&vms_node_undealed, _day_id);
		for (int i = 0; i < vms_node_undealed.size(); i++)
		{
			addVm2Ser(vms_node_undealed.at(i), true, _day_id);
		}*/


		//将剩下的虚拟机放入服务器  先双后单
		for (int j = 0; j < vms_node_d.size(); j++)
		{
			if (vms_node_d.at(j)->dealed == false)
				addVm2Ser(vms_node_d.at(j),true, _day_id);
		}
		for (int j = 0; j < vms_node_s.size(); j++)
		{
			if (vms_node_s.at(j)->dealed == false)
				addVm2Ser(vms_node_s.at(j), true, _day_id);
		}

		//处理剩下的删除请求
		for (auto it = del_msg_day.begin(); it != del_msg_day.end(); ++it)
		{
			if(del_msg_day.at(it->first).dealed == false)
				delVm(&del_msg_day.at(it->first), _day_id);
		}
		//服务器排序
		own_sers.sort();
	}

	void migration(vector<MigrationMsg> *mig_msg,int _day_id)
	{
		unordered_map<int,OwnServer*> mig_sers; //迁移的服务器或者空闲的 之后就不往里面迁移了
		int mig_num=0; //迁移次数
		if (own_sers.using_ser.size() == 0) return;
		for (auto it = --own_sers.using_ser.end(); it != own_sers.using_ser.begin(); it--)
		{
			if (mig_num >= floor( 5*vms_ser.size() /1000 -2 ) )
				break;
			if (it->usage_cpu ==0 || it->usage_mem ==0)
			{
				mig_sers[it->id] = &*it;
				continue; //利用率为0就不考虑了
			}
			if (it->usage_cpu < 0.2 || it->usage_mem < 0.2  ) //利用率低
			{
				mig_sers[it->id] = &*it;
				vector<VM2Server*> vms_remove;
				for (auto it_vm = it->vms.begin(); it_vm != it->vms.end(); ++it_vm)
				{
					if (mig_num >= floor( 5*vms_ser.size() /1000 -2 ) )
						break;
					bool inset_success = false;
					for (auto it2 = own_sers.using_ser.begin(); it2 != own_sers.using_ser.end(); ++it2)
					{
						if(mig_sers.find(it2->id) == mig_sers.end())   //不在迁移表中
						{
							if (mig_num >= floor( 5*vms_ser.size() /1000 -2 ) )
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
								mig_num++;
								break;
							}
						}
					}
				}
				for(int i=0 ;i< vms_remove.size() ;i++)
				{
					it->removeVM(vms_remove.at(i),vms_remove.at(i)->a_b_mig , _day_id);
				}
			}
		}
		//cout << "mig_num: "<< mig_num << "  "<< vms_ser.size() <<"  "<< vms_ser.size()*5 /1000 <<"\n";
	}

	void  simulatedFire(vector<VM2Server*> *vms , int _day_id)
	{
		const int max_nums = 500;  //最大运行次数

		double r = 0.98;	 //用于控制降温的快慢
		double T = 30000;		 //系统的温度，系统初始应该要处于一个高温的状态
		double T_min = 0.1; //温度的下限，若温度T达到T_min，则停止搜索
		int cost_ori;
		int best,current;
		int dE;  //current与best差值
		vector<ServersData> sers_ori;
		vector<ServersData> sers_copy, sers_copy_best;

		cost_ori = calSersCost(vms, &sers_ori);
		//copy数据
		sers_copy = sers_ori;
		sers_copy_best = sers_copy;
		
		srand((unsigned)(time(NULL)));

		best = cost_ori;
		//cout << "before fire:" << cost_ori << endl;

		while (T > T_min)
		{
			generateNew(&sers_copy, 50);
			current = calSersCost(vms, &sers_copy);
			dE = current - best;
			if (current < best) //移动后得到更优解，则总是接受移动
			{
				best = current;
				//将新的顺序赋值
				sers_copy_best = sers_copy;
			}
			else
			{
				// 函数exp( dE/T )的取值范围是(0,1) ，dE/T越大，则exp( dE/T )也越大
				//一定的概率接受差的值
				if (exp( -dE / T ) > rnd(0.0, 1.0))
				{
					best = current;
					sers_copy_best = sers_copy;
				}
			}
			T = r * T; //降温退火 ，0<r<1 。r越大，降温越慢；r越小，降温越快
		}
		//cout << "after fire:" << best << endl;

		//如果运行后小于原来值  则赋值
		if (best < cost_ori)
		{
			sers_ori = sers_copy_best;
		}
		for (int i = 0; i < sers_ori.size(); i++)
		{
			own_sers.addSer(sers_ori.at(i), _day_id);
		}
	}

	//计算成本
	int calSersCost(vector<VM2Server*> *vms, vector<ServersData> *_sers_list)
	{
		vector<OwnServer> sers_add;
		int cost=0;
		for (int i = 0; i < _sers_list->size(); i++)
		{
			OwnServer ser(0, _sers_list->at(i));
			sers_add.push_back(ser);
		}
		for (int i = 0; i < vms->size(); i++)
		{
			//在现在的列表中添加虚拟机  如果没有空间，则申请当天所选的服务器
			addVmsDaySers(vms->at(i), &sers_add, &day_ser_select, 0);
		}
		//更新服务器表
		_sers_list->clear();
		for (auto it = sers_add.begin(); it != sers_add.end(); it++)
		{
			if (it->unused == false)
			{
				cost += it->ser.hardware_cost;
				_sers_list->push_back(it->ser);
			}
		}
		return cost;
		
	}

	//产生新解：随机往数组里放新的服务器
	void generateNew(vector<ServersData> *sers,int swap_times)
	{
		for (int i = 0; i < swap_times; i++)
		{
			auto random_it = std::next(std::begin(data_hand->servers), rand() % (data_hand->servers.size()));
			int posx = rand() % sers->size();  //产生0-vms->size() 的随机数
			sers->at(posx) = data_hand->servers.at(random_it->first);
		}
	}

	//产生（dbLow,dbUpper）之间的随机数
	double rnd(double dbLow, double dbUpper)
	{
		double dbTemp = rand() / ((double)RAND_MAX + 1.0);
		return dbLow + dbTemp * (dbUpper - dbLow);
	}

	//查找请求中最大cpu与mem
	void findMaxCpuMem(vector<VM2Server> *vms , int *_max_cpu ,int *_max_mem)
	{
		*_max_cpu = 0; *_max_mem = 0;
		for (int i = 0; i < vms->size(); i++)
		{
			if (vms->at(i).vm.node == 0) //单节点cpu memory 按两倍算
			{
				if (*_max_cpu < 2 * vms->at(i).vm.cpu)
					*_max_cpu = 2 * vms->at(i).vm.cpu;
				if (*_max_mem < 2 * vms->at(i).vm.memory)
					*_max_mem = 2 * vms->at(i).vm.memory;
			}
			else
			{
				if (*_max_cpu < vms->at(i).vm.cpu)
					*_max_cpu = vms->at(i).vm.cpu;
				if (*_max_mem < vms->at(i).vm.memory)
					*_max_mem = vms->at(i).vm.memory;
			}
		}
	}


	//根据每天没有处理过的虚拟机数据总和 选择服务器
	void selectSer(DayRequestData *dat_req, int _day_id)
	{
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
			_num += 5;
			//数量 * 硬件成本加剩余天数的使用成本
			int it_cost = _num * ( data_hand->servers.at(it->first).hardware_cost + (all_day_num - _day_id) * data_hand->servers.at(it->first).energy_day) ;

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
	void addVm2Ser(VM2Server *_vm2ser,bool add_new_ser, int _day_id)
	{
		bool inset_success = false;
		
		//判断使用中服务器中是否有空闲位置
		for (auto it = own_sers.using_ser.begin(); it != own_sers.using_ser.end(); it++)
		{
			inset_success = it->insertVM( _vm2ser, _day_id);
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
			else
			{
				own_sers.addSer(max_cpu_ser, _day_id);  //添加服务器
				inset_success2 = own_sers.using_ser.back().insertVM(_vm2ser, _day_id);
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
			inset_success = it->insertVM(_vm2ser,  _day_id);
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
			vms_ser[del_msg->id].own_ser->removeVM(&vms_ser[del_msg->id],vms_ser[del_msg->id].a_b ,_day_id);
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
		cout << "(migration, " << mig_msgs_day.size()<< ")\n";
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

	}

	void cout2File(ofstream &out_file,int _day_id)
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

	void coutAllCosts()
	{
		int all_costs=0 , hard_cost=0;
		for (auto iter = own_sers.using_ser.begin(); iter != own_sers.using_ser.end(); iter++)
		{
			iter->calCosts(all_day_num);
			all_costs += iter->cost_all;
			hard_cost += iter->ser.hardware_cost;
		}
		cout << "costs:"<< all_costs <<"\n";
		cout << "hard_cost:" << hard_cost << "\n";
		cout << "all_day_use_cost:" << all_costs - hard_cost << "\n";

		cout << "all_sers_num :" << own_sers.using_ser.size() <<"\n";
	}
	
private:
	DataHandling *data_hand;
	AllServers own_sers;  //拥有的所有服务器              
    unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器  <id,对应服务器>
	ServersData max_cpu_ser, max_mem_ser, min_hardcost_ser;
	ServersData day_ser_select;    //当天选择的服务器类型
	int max_cpu, max_mem;  //每天请求中最大cpu与最大mem
	int all_day_num; //总天数
	vector<MigrationMsg> mig_msgs_day;  //每天的迁移
	
};



#endif