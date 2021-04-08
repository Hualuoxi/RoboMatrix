#include <iostream>
#include "data_handling.hpp"
#include "strategy.hpp"
#include <thread>
#include <mutex>
void* read_Req(void* args);
mutex mut;


int main(int argc, char **argv)
{
	DataHandling *data_handling = new DataHandling(false);
	Strategy *strategy = new Strategy(data_handling);
	int deal_day_id = 0;
	//ofstream out_file("output.txt", ios::trunc);
#ifdef DEBUG
	data_handling->openFile("/home/hualuoxi/Desktop/CodeCraftRe/RoboMatrix/training-1.txt");
	ofstream out_CSV("sersUsage.csv", ios::trunc);
	out_CSV << "_day_id" <<","<< "allMigNum" <<","
			    << "mig_num_day"<<"," << "migByUsageNumDay"<<"," << "migByCpuMemDay"<<"," << "migByEnergyDay"<<","
				<<"delRat"<<","
				<< "own_sersNum)" << "," << "unUsedSers" <<","
				<< endl;
#else
	pthread_t tids;
	int ret = pthread_create(&tids, NULL, read_Req, (void*)data_handling);
	if(0 != ret)
		cout << "pthread_create error: error_code=" << ret << endl;
#endif
	//读取所有数据
	while(true)
	{
		mut.lock();
		if(data_handling->readed_kday_reqs &&
		((data_handling->day_tmp-1) > deal_day_id || (((data_handling->day_tmp-1) == deal_day_id) && data_handling->day_read_finished)))
		{
			mut.unlock();
			strategy->dealDayReq(&data_handling->requests_all->at(deal_day_id), deal_day_id);
			#ifdef DEBUG
				// strategy->coutMsg2CSV(out_CSV,deal_day_id);
				// if(deal_day_id <= 100)
				// 	strategy->coutDayUsage2CSV(out_CSV ,deal_day_id);
				//strategy->coutAllSersUsage();
			#else
				strategy->coutDayMsg(deal_day_id);
			#endif
			
			//strategy->cout2File(out_file,deal_day_id);
			deal_day_id++;
		}
		mut.unlock();
		mut.lock();
		if (deal_day_id && deal_day_id == data_handling->day_num  )   //处理完所有数据 
			break;
		mut.unlock();
	}
#ifdef DEBUG
	strategy->coutAllCosts();
#endif
	//strategy->coutAllSersUsage();
	//out_file.close();	
	return 0;
}


void* read_Req(void* args)
{
	// cout << "this is thread read_Req" << endl;
	DataHandling* data_handling = nullptr;
	data_handling = (DataHandling*)args;
	string str_line;
	while (!data_handling->read_all_finished)
	{
		getline(cin, str_line);
		mut.lock();
		data_handling->dealLineData(str_line);
		mut.unlock();
	}
	pthread_exit(NULL);
    return 0;
}