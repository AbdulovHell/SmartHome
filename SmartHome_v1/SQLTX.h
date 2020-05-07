#ifndef _SQLTX_H_
#define _SQLTX_H_

#include "threading.h"

namespace Threading {
	using namespace std;	

	class Info {

	public:
		virtual void Send() = 0;
	};

	class SoundInfo : virtual public Info {
		int Act;

	public:
		SoundInfo(int a);

		void Send();
	};

	class MovingInfo : virtual public Info {
		int Act;
	public:
		MovingInfo(int a);

		void Send();
	};

	class MeasInfo : virtual public Info {
		float _temp, _hum, _light, _press;
	public:
		MeasInfo(float temp, float hum, float light, float press);

		void Send();
	};

	void SendInfo(unique_ptr<Info> inf);

	class SQLTXThread : virtual public Thread {
	private:
		static void* SQLThread(void* ptr_null);
	protected:
	public:
		SQLTXThread() : Thread::Thread(SQLThread, (void*)NULL) { }
		~SQLTXThread();
	};
}

#endif //_SQLTX_H_