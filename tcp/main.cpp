#include "device.h"
#include "Process.h"

int main()
{
	Device A, B;
	Process* processA = A.createProcess();
	Process* processB = B.createProcess();
	processA->bindPort(4032);
	processB->bindPort(5056);
	if (processA->connect(&B, 5056))
	{
		processA->run();
	}
	system("pause");
	return 0;
}