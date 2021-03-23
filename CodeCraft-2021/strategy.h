
#ifndef _STRATEGY_H
#define _STRATEGY_H

#include "data_handling.h"
#include <list>
#include "math.h"
#include <cmath>
//#include <algorithm>

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
	OwnServer(int _id, ServersData _ser);

	//将虚拟机放入服务器  _a_b放入的节点 （-1 双节点    0 a  1 b）
	bool insertVM(VMData _vm_data ,int *_a_b ,int _day_id);
	//去掉虚拟机  _a_b放入的节点 （-1 双节点    0 a  1 b）
	void removeVM(VMData _vm_data,int _a_b, int _day_id);
	//打印利用率
	void coutUsage();
	//计算总成本 
	void calCosts(int _day_id);
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
	vector<VM2Server*> matchs_k; //匹配的型号 相同类型的节点才会匹配
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
	void addSer(ServersData _ser_dat , int _day_id);
};


class Strategy
{
private:
	
	AllServers own_sers;  //拥有的所有服务器              
	unordered_map<int, VM2Server> vms_ser;    //所有虚拟机及对应服务器  <id,对应服务器>
	ServersData max_cpu_ser, max_mem_ser, min_hardcost_ser;
	ServersData day_ser_select;    //当天选择的服务器类型
	int max_cpu, max_mem;  //每天请求中最大cpu与最大mem
	int all_day_num; //总天数
	int undealed_num;  //每天未处理的请求
public:
	DataHandling *data_hand;
	Strategy(void);

	void dealDayReq(DayRequestData *dat_req, int _day_id);
	void simulatedFire();
	//根据每天没有处理过的虚拟机数据总和 选择服务器
	void selectSer(DayRequestData *dat_req, int _day_id);
	//查找与每天相匹配的型号 
	
	void findMatchsDay(DayRequestData *dat_req);
	
	void addVm2Ser(VM2Server *_vm2ser,bool add_new_ser, int _day_id);
	//将虚拟机添加到尾部服务器
	void addVmsSerBack(VM2Server *_vm2ser, int _day_id);
	//同时添加匹配的虚拟机到已有服务器  没有空间则申请新的服务器
	void addMatchVms2ser(VM2Server *_vm2ser,int _day_id);
	//删除虚拟机
	void delVm(RequestData _req, int _day_id);
	//冒泡排序  按cpu与mem的和  由大到小排序
	void bubble(vector<VM2Server*> &vms);
	//输出每日信息
	void coutDayMsg(int _day_id);
	void coutAllSersUsage();
	void coutAllCosts();
};



#endif