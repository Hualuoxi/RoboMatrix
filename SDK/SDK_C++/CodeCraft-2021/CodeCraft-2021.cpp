#include "iostream"
#include "data_handling.hpp"

int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	DataHandling *data_handling = new DataHandling(false);
	// data_handling->openFile("training-1.txt");

	//读取所有数据
	string str_line;
	while(getline(cin,str_line))
	{
		//cout<<"str_line.length: "<< str_line.length()<<endl;
		if(str_line.length() > 0)
			if(data_handling->dealLineData(str_line))
				break;
	}

	int id =0;
	//cout << "\n";
	//每天的输出信息
	for(int i=0; i < data_handling->requests_all->size();i++)
	{
		//购买型号的数量
		cout << "(purchase, "<< 1 <<")\n";  
		//型号及其对应购买数量
		cout << "(hostGCX19, "<< data_handling->requests_all->at(i).day_request.size() <<")\n";
		//迁移数量
		cout << "(migration, " <<0 << ")\n";

		for(int j=0;j<data_handling->requests_all->at(i).day_request.size();j++)
		{
			if(data_handling->requests_all->at(i).day_request.at(j).req == "add")
			{
				//单节点
				if(data_handling->vms.at( data_handling->requests_all->at(i).day_request.at(j).vm_type ).single ==1 )
				{
					cout <<"("<<j<<", A)\n";  //A or B
				}
				else
				{
					cout <<"("<<j<<")\n";
				}
			}
			
		}
		//cout<<"\n";
	}

	return 0;
}
