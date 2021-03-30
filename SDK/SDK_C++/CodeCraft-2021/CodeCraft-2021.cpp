#include "iostream"
#include "data_handling.hpp"
#include "strategy.hpp"

int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	DataHandling *data_handling = new DataHandling(false);
	//data_handling->openFile("training-1.txt");

	Strategy *strategy = new Strategy(data_handling);
	int deal_day_id = 0;
	//ofstream out_file("output.txt", ios::trunc);

	//读取所有数据
	string str_line;
	while(true)
	{
		while (!data_handling->read_all_finished && getline(cin, str_line) )
		{
			if (str_line.length() > 0)
			{
				if (!data_handling->dealLineData(str_line))
				{
					if (!data_handling->readed_kday_reqs) continue;  //没有读完k天的数据
					if (data_handling->day_read_finished)    //读完当天的数据
						break;   //跳出读取循环
				}
				else
					break;
			}
		}

		strategy->dealDayReq(&data_handling->requests_all->at(deal_day_id), deal_day_id);
		strategy->coutDayMsg(deal_day_id);
		//strategy->cout2File(out_file,deal_day_id);
		deal_day_id++;
		if (deal_day_id == data_handling->requests_all->size())  //处理完所有数据 
			break;
	}
	//strategy->coutAllSersUsage();
	//out_file.close();
	
	//system("pause");
	return 0;
}
