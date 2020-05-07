#ifndef _TCP_H_
#define _TCP_H_

#include "threading.h"

namespace Threading {
	class TCPReciverThread;

	extern std::vector<Threading::TCPReciverThread*> Listeners;
	extern std::mutex* ListenersMutex;

	//
#define READ 1
#define WRITE 2
	//0x1F
#define FREQ 0x1
#define RFATT 0x2
#define IFATT 0x4
#define _IF 0x8
#define REF 0x10
	//
#define DEBUG_ATT 0x20
#define RESERVED 0x40
#define TEST_CONN 0x80

	struct TRData {
		uint32_t Freq;
		uint8_t RFatt;
		uint8_t IFatt;
		uint8_t IF;
		uint8_t Ref;
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
}

#endif
