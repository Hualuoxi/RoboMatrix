#include <iostream>
#include "data_handling.hpp"
#include "strategy.hpp"
#include <thread>
#include <mutex>
void* read_Req(void* args);
mutex mut;
int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	DataHandling *data_handling = new DataHandling(false);
	// data_handling->openFile("training-1.txt");
	Strategy *strategy = new Strategy(data_handling);
	int deal_day_id = 0;
	//ofstream out_file("output.txt", ios::trunc);
	pthread_t tids;
	int ret = pthread_create(&tids, NULL, read_Req, (void*)data_handling);
	if(0 != ret)
		cout << "pthread_create error: error_code=" << ret << endl;

	//读取所有数据
	while(true)
	{
		mut.lock();
		if(data_handling->readed_kday_reqs &&
		((data_handling->day_tmp-1) > deal_day_id || (((data_handling->day_tmp-1) == deal_day_id) && data_handling->day_read_finished)))
		{
			mut.unlock();
			strategy->dealDayReq(&data_handling->requests_all->at(deal_day_id), deal_day_id);
			strategy->coutDayMsg(deal_day_id);
			//strategy->cout2File(out_file,deal_day_id);
			deal_day_id++;
		}
		mut.unlock();


		mut.lock();
		if (deal_day_id && deal_day_id == data_handling->day_num  )   //处理完所有数据 
			break;
		mut.unlock();
	}
	//strategy->coutAllCosts();

	// string str_line;
	// while(getline(cin,str_line))
	// {
	// 	//cout<<"str_line.length: "<< str_line.length()<<endl;
	// 	if(str_line.length() > 0)
	// 		if(data_handling->dealLineData(str_line))
	// 			break;
	// }
	// for (int i = 0; i < data_handling->requests_all->size(); i++)
	// {
	// 	strategy->dealDayReq(&data_handling->requests_all->at(i), i);
	// 	strategy->coutDayMsg(i);
	// }
	// strategy->coutAllCosts();
	
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