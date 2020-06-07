#ifndef _TCP_H_
#define _TCP_H_

#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <mutex>

//class Thread;
//class TCPServerThread;
class TCPReciverThread;

extern std::vector<TCPReciverThread*> Listeners;
extern std::mutex* ListenersMutex;

struct TimeATemp {
	char h_or_d;
	char m_or_M;
	float temp;
};

class Thread {
protected:
	pthread_t threadHandle;
	struct RTData {
		bool* bStopFlag;
		int iSocket;
	};
public:
	Thread() {}
	Thread(void*(func)(void*), void* arg);
	~Thread();
	pthread_t GetThrdHandle();
	int Join();
};

class TCPServerThread : virtual public Thread {
private:
	static void* SocketServer(void* threadID);
protected:
public:
	TCPServerThread(int _port) : Thread(SocketServer, (void*)_port) {}
	~TCPServerThread();
};

class TCPReciverThread : virtual public Thread {
private:
	static void* Recive(void* threadID);
	RTData dt;
protected:

public:
	bool Stoped = false;
	TCPReciverThread(int _sock) {
		dt.bStopFlag = &Stoped;
		dt.iSocket = _sock;
		pthread_create(&threadHandle, NULL, Recive, (void*)&dt);
	}
	~TCPReciverThread();
};

#endif
