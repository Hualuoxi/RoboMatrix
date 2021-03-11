#include "iostream"
#include "data_handling.hpp"

int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	DataHandling *data_handling = new DataHandling(false);
	data_handling->openFile("SDK/SDK_C++/CodeCraft-2021/training-1.txt");

	
	cout << data_handling->servers.at("hostGCX19").hardware_cost << "  \n";
	cout << data_handling->requests_all->size()<<"  \n";
	for(int i=0; i < data_handling->requests_all->size();i++)
	{
		for(int j=0;j<data_handling->requests_all->at(i).day_request.size();j++)
		{
			cout<<data_handling->requests_all->at(i).day_request.at(j).id <<"\n";
		}
		cout<<"\n";
	}
	cout <<"finished\n";

	return 0;
}
