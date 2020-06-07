#include "TCP.h"

using namespace std;

bool Working = true;
TCPServerThread* thrd;
const int ServerPort = 9892;

void InitShutdown() {
	Working = false;

	thrd->~TCPServerThread();

	for (size_t i = 0; i < Listeners.size(); i++) {
		Listeners[i]->~TCPReciverThread();
	}
}

int main(int argc, char* argv[])
{
	ListenersMutex = new mutex();
	thrd = new TCPServerThread(ServerPort);

	while (Working)
	{
		this_thread::sleep_for(std::chrono::milliseconds(100));

		ListenersMutex->lock();
		vector<TCPReciverThread*>::iterator itRecv = Listeners.begin();
		for (size_t i = 0; i < Listeners.size(); i++, itRecv++) {
			if (Listeners[i]->Stoped) {
				Listeners.erase(itRecv);
			}
		}
		ListenersMutex->unlock();
	}
	return 0;
}