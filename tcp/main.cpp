#include "device.h"
#include "Process.h"

#include <Windows.h>

int main()
{
	Device A, B;
	Process* processA = A.createProcess();
	Process* processB = B.createProcess();
	processA->bindPort(4032);
	processB->bindPort(5056);
	if (processA->connect(&B, 5056))
	{
		std::thread thread1(&Process::write, processA);
		thread1.join();
		std::thread thread2(&Process::read, processB);
		thread2.join();
	}
	return 0;
}