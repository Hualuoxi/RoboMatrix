#include "iostream"
#include "data_handling.hpp"
#include "simple_schedule.hpp"

int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	DataHandling *data_handling = new DataHandling(false);
	Schedule *simple_schedule = new Schedule(false);
	// data_handling->openFile("training-3.txt");

	//读取所有数据
	string str_line;
	while(getline(cin,str_line))
	{
		if(str_line.length() > 0)
			if(data_handling->dealLineData(str_line))
				break;
	}

	// unordered_map  <string, ServersData>::iterator iter_server;
	// for (iter_server = data_handling->servers.begin(); iter_server != data_handling->servers.end(); iter_server++)
	// {
	// 	ServersData server_data;
	// 	iter_server->second;
	// }
	unordered_map  <string, ServersData>::iterator iter_server = data_handling->servers.begin();
	ServersData server = iter_server->second;
	//每天的输出信息
	for(long unsigned int i = 0; i < data_handling->requests_all->size(); i++)
	{
		cout << simple_schedule->simple_schedule(data_handling->requests_all->at(i), server, data_handling->vms);
	}
	

	return 0;
}
