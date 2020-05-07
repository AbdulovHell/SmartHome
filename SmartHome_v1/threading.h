#ifndef _THREADING_H_
#define _THREADING_H_

namespace Threading {
	using namespace std;	

	//void * Timing(void * ptr_null);
	//extern float Temperature;
	//extern bool UpdateNow;

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

	class TimingThread : virtual public Thread {
	private:
		static void* Timing(void* ptr_null);
	protected:
	public:
		TimingThread() :Thread(Timing, (void*)NULL) { }
		~TimingThread();
	};
}
#endif	//#ifndef _THREADING_H_
