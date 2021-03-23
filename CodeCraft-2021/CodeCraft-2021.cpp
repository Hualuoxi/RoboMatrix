#include "CodeCraft-2021.h"
int main(int argc, char **argv)
{
	// TODO:read standard input
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	Strategy *strategy = new Strategy();

	for (unsigned int i = 0; i < strategy->data_hand->requests_all->size(); i++)
	{
		strategy->dealDayReq(&strategy->data_hand->requests_all->at(i), i);
		strategy->coutDayMsg(i);
	}

	//strategy->coutAllSersUsage();
	//system("pause");
	return 0;
}
