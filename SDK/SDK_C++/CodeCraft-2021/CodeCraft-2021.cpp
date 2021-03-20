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
	
	//读取所有数据
	string str_line;
	while(getline(cin,str_line))
	{
		//cout<<"str_line.length: "<< str_line.length()<<endl;
		if(str_line.length() > 0)
			if(data_handling->dealLineData(str_line))
				break;
	}

	Strategy *strategy = new Strategy(data_handling);

	for (int i = 0; i < data_handling->requests_all->size(); i++)
	{
		strategy->dealDayReq(&data_handling->requests_all->at(i), i);
		strategy->coutDayMsg(i);
	}

	//strategy->coutAllSersUsage();
	//system("pause");
	return 0;
}
