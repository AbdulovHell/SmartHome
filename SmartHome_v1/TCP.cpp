#include "sys_headers.h"

#include "Colorize.h"
#include "stuff.h"
#include "TCP.h"
#include "main.h"
#include "Task.h"
#include "I2C.h"

namespace Threading {
	using namespace Stuff;

	std::vector<Threading::TCPReciverThread*> Listeners;
	std::mutex* ListenersMutex;

	void* TCPServerThread::SocketServer(void * param)
	{
		uint16_t port = (uint16_t)(int)param;
		int listener = 0;
		struct sockaddr_in addr;
		printf("%s: Server start on %d port\n", Stuff::MakeColor("SERVER", Stuff::Red).c_str(), port);
		listener = socket(AF_INET, SOCK_STREAM, 0);
		if (listener < 0)
		{
			perror("socket");
			exit(1);
		}

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			perror("bind");
			exit(2);
		}

		listen(listener, 5);
		int tempsock;

		while (1) {
			tempsock = accept(listener, NULL, NULL);
			if (tempsock < 0) {
				perror("accept");
				exit(3);
			}
			printf("%s: Accept connection\n", Stuff::MakeColor("SERVER", Stuff::Red).c_str());
			ListenersMutex->lock();
			Listeners.push_back(new TCPReciverThread(tempsock));
			ListenersMutex->unlock();
		}
	}

	TCPServerThread::~TCPServerThread()
	{
		pthread_cancel(threadHandle);
	}

	void * TCPReciverThread::Recive(void * param)
	{
		RTData* data = (RTData*)param;
		int reciver = data->iSocket;
		bool* stopflag = data->bStopFlag;
		char buf[128];
		int bytes_read;
		printf("%s(%d): Start working\n", Stuff::MakeColor("RECV", Stuff::Yellow).c_str(), reciver);
		while (1) {
			//this_thread::sleep_for(std::chrono::microseconds(1));
			bytes_read = recv(reciver, buf, 128, 0);
			if (bytes_read <= 0) break;
			//проверка
			//if (!Stuff::Verify(buf)) break;
			//обработка
			if (buf[0] < 0x20) {
				if (buf[1] == READ) {
					TRData dt;
					memset(&dt, 0, sizeof(TRData));

					/*dt.Freq = Stuff::Storage->GetFreq();
					dt.IF = Stuff::Storage->GetIF();
					dt.IFatt = Stuff::Storage->GetIFAtt();
					dt.RFatt = Stuff::Storage->GetRFAtt();
					dt.Ref = Stuff::Storage->GetRef();*/

					memcpy(buf + 2, &dt, sizeof(TRData));
					send(reciver, buf, 128, 0);
					printf("%s(%d): Send TRData\n", Stuff::MakeColor("RECV", Stuff::Yellow).c_str(), reciver);
				}
				else if (buf[1] == WRITE) {
					TRData dt;
					memcpy(&dt, buf + 2, sizeof(TRData));
				
					if (buf[0] & FREQ) Threading::AddTask(make_unique<Threading::TaskSetFreq>(dt.Freq));
					if (buf[0] & RFATT || buf[0] & IFATT) Threading::AddTask(make_unique<Threading::TaskSetAtt>(dt.RFatt, dt.IFatt));
					if (buf[0] & _IF) Threading::AddTask(make_unique<Threading::TaskSetOutput>(dt.IF));
					//if (buf[0] & REF) Threading::AddTask(make_unique<Threading::TaskChangeRef>(dt.Ref));

					sprintf(buf + 1, "Ok. F:%d RFatt:%d IFAtt:%d %s\n", dt.Freq, dt.RFatt, dt.IFatt, dt.IF ? "1485" : "140");
					buf[0] = RESERVED;
					send(reciver, buf, 128, 0);

					printf("%s(%d): TRData accepted\n", Stuff::MakeColor("RECV", Stuff::Yellow).c_str(), reciver);
				}
			}
			else
				switch (buf[0]) {	//type id			
				case DEBUG_ATT:	//debug att
				{
					if (buf[1] == READ) {
						IO::I2C dev(0x24);	//b3-8
						if (dev.IsOpen()) {
							for (int i = 0; i < 16; i++) {
								buf[i + 2] = dev.ReadByteFromReg(i + 8);
							}
						}
						send(reciver, buf, 128, 0);
					}
					else if (buf[1] == WRITE) {
						IO::I2C dev(0x24);//b3-8
						if (dev.IsOpen()) {
							for (int i = 0; i < 16; i++) {
								dev.WriteReg((uint8_t)buf[i + 2], i + 8);
							}
							dev.WriteReg((uint8_t)2, 0); //команда на обновление eeprom
						}
						sprintf(buf + 1, "Debug att written to b3-8\n");
						buf[0] = RESERVED;
						send(reciver, buf, 128, 0);
					}
				}
				break;
				case RESERVED:
				{

				}
				break;
				case TEST_CONN:
				{
					sprintf(buf + 1, "Connected\n");
					buf[0] = RESERVED;
					send(reciver, buf, 128, 0);
				}
				break;
				default:
					break;
				}
		}
		printf("%s(%d): Stop working\n", Stuff::MakeColor("RECV", Stuff::Yellow).c_str(), reciver);
		*stopflag = true;
		close(reciver);
		return 0;
	}

	TCPReciverThread::~TCPReciverThread()
	{
		pthread_cancel(threadHandle);
	}
}