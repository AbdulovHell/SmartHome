#include "sys_headers.h"
#include <mariadb/mysql.h>
#include "threading.h"
#include "SQLTX.h"
#include "main.h"

namespace Threading {

	using namespace std;

	class Info;
	class SoundInfo;
	class MovingInfo;
	class MeasInfo;
	
	std::vector<std::unique_ptr<Info>> InfoToSend;
	std::mutex* SendInfoMutex;
	MYSQL* hDB = NULL;

	SoundInfo::SoundInfo(int a) {
		Act = a;
	}
	int SoundInfo::Send() {
		char buf[200];
		sprintf(buf, "INSERT INTO Sound (Activity) VALUES (%d);", Act);
		return mysql_query(hDB, buf);
	}

	MovingInfo::MovingInfo(int a) {
		Act = a;
	}
	int MovingInfo::Send() {
		char buf[200];
		sprintf(buf, "INSERT INTO Moving (Activity) VALUES (%d);", Act);
		return mysql_query(hDB, buf);
	}

	MeasInfo::MeasInfo(float temp, float hum, float light, float press) {
		_temp = temp;
		_hum = hum;
		_light = light;
		_press = press;
	}
	int MeasInfo::Send() {
		char buf[200];
		sprintf(buf, "INSERT INTO Measures (Temperature, Humidity,Pressure, LightLvl) VALUES (%.2f,%.2f,%.2f,%.2f);", _temp, _hum,_press, _light);
		return mysql_query(hDB, buf);
	}

	void SendInfo(unique_ptr<Info> inf)
	{
		SendInfoMutex->lock();
		InfoToSend.push_back(std::move(inf));
		SendInfoMutex->unlock();
	}
	
	void * SQLTXThread::SQLThread(void * ptr_null)
	{
		SendInfoMutex = new std::mutex();

		hDB = mysql_init(NULL);
		hDB = mysql_real_connect(hDB, NULL, "root", NULL, "SM_DATA", 3306, NULL, CLIENT_IGNORE_SPACE | CLIENT_MULTI_STATEMENTS);
		if (hDB == NULL) {
			WriteLog("SQLThread", "mysql_real_connect() return NULL.", Stuff::Red);
			mysql_close(hDB);
		}
		//mysql_query(hDB, "INSERT INTO books (Title, SeriesID, AuthorID) VALUES (\"Mehovaya bestoloch\", 7, 7);");
		while (1) {
			SendInfoMutex->lock();
			int size = InfoToSend.size();
			if (size > 0) {
				unique_ptr<Info> temp = move(InfoToSend[0]);
				InfoToSend.erase(InfoToSend.begin());
				SendInfoMutex->unlock();
				int res=temp->Send();
				if(res!=0) WriteLog("SQLThread", "mysql_query() error", Stuff::Red);
			}
			else
				SendInfoMutex->unlock();

			this_thread::sleep_for(chrono::milliseconds(10));
		}
		return nullptr;
	}

	SQLTXThread::~SQLTXThread()
	{
		pthread_cancel(threadHandle);
	}

}