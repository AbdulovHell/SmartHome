#include <mariadb/mysql.h>
#include "TCP.h"

std::vector<TCPReciverThread*> Listeners;
std::mutex* ListenersMutex;

using namespace std;

Thread::Thread(void*(func)(void*), void* arg)
{
	pthread_create(&threadHandle, NULL, func, arg);
}

Thread::~Thread()
{
	pthread_cancel(threadHandle);
}

pthread_t Thread::GetThrdHandle()
{
	return threadHandle;
}

int Thread::Join()
{
	return pthread_join(threadHandle, NULL);
}

void* TCPServerThread::SocketServer(void * param)
{
	uint16_t port = (uint16_t)(int)param;
	int listener = 0;
	struct sockaddr_in addr;
	printf("SERVER: Start on %d port\n", port);
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
		printf("SERVER: Accept connection\n");
		ListenersMutex->lock();
		Listeners.push_back(new TCPReciverThread(tempsock));
		ListenersMutex->unlock();
	}
}

TCPServerThread::~TCPServerThread()
{
	pthread_cancel(threadHandle);
}

int GetLastMeasure(int reciver) {
	MYSQL* hDB = mysql_init(NULL);
	hDB = mysql_real_connect(hDB, NULL, "reader", "reader", "SM_DATA", 3306, NULL, CLIENT_IGNORE_SPACE | CLIENT_MULTI_STATEMENTS);
	if (hDB == NULL) {
		printf("RECV(%d):SQL: mysql_real_connect() return NULL.\n", reciver);
		printf("RECV(%d):SQL: Error message:\n");
		printf(mysql_error(hDB));
		printf("----------------------------\n");
		mysql_close(hDB);
		return 0;
	}
	else {
		mysql_query(hDB, "SELECT * FROM Measures ORDER BY DateTime DESC LIMIT 1;");
		MYSQL_RES* res = mysql_store_result(hDB);
		uint fields = mysql_field_count(hDB);
		printf("RECV(%d):SQL: field count %d\n", reciver, fields);
		MYSQL_ROW row = mysql_fetch_row(res);
		//printf(row[0]); //DateTime
		//printf(row[1]); //Temp
		//printf(row[2]); //Hum
		//printf(row[3]); //Press
		//printf(row[4]); //Light
		float temp = strtof(row[1], NULL);
		float hum = strtof(row[2], NULL);
		float press = strtof(row[3], NULL);
		float light = strtof(row[4], NULL);

		char buf[32] = { 0, };

		buf[0] = 0xAA;
		buf[1] = 0x55;
		buf[2] = 0x5;

		memcpy(buf + 3, &temp, 4);
		memcpy(buf + 7, &hum, 4);
		memcpy(buf + 11, &press, 4);
		memcpy(buf + 15, &light, 4);

		mysql_close(hDB);
		write(reciver, buf, 32);
		return 32;
	}
}

int PackTemperatureData(char mode, int reciver) {
	MYSQL* hDB = mysql_init(NULL);
	hDB = mysql_real_connect(hDB, NULL, "reader", "reader", "SM_DATA", 3306, NULL, CLIENT_IGNORE_SPACE | CLIENT_MULTI_STATEMENTS);

	switch (mode) {
	case 1:	//30 дней
	{
		mysql_query(hDB, "SELECT DATE_FORMAT(DateTime,\"%d.%m\"),AVG(Temperature) FROM Measures WHERE MONTH(DateTime)=MONTH(CURDATE())-1 OR MONTH(DateTime)=MONTH(CURDATE())  GROUP BY DAYOFYEAR(DateTime) ORDER BY DateTime DESC LIMIT 30;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Temperature(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount - 1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string d_s = dt.substr(0, 2);
			string M_s = dt.substr(3, 2);
			int d = atoi(d_s.c_str());
			int M = atoi(M_s.c_str());
			data[i].h_or_d = d;
			data[i].m_or_M = M;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Temperature(mode=%d): %d.%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x10;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;

	default:
	case 0:	//За последние сутки
	{
		mysql_query(hDB, "SELECT TIME_FORMAT(DateTime,\"%H:%i\"),Temperature from Measures order by DateTime desc LIMIT 96;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Temperature(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount-1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string h_s = dt.substr(0, 2);
			string m_s = dt.substr(3, 2);
			int h = atoi(h_s.c_str());
			int m = atoi(m_s.c_str());
			data[i].h_or_d = h;
			data[i].m_or_M = m;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Temperature(mode=%d): %d:%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x10;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;
	}
}

int PackHumidityData(char mode, int reciver) {
	MYSQL* hDB = mysql_init(NULL);
	hDB = mysql_real_connect(hDB, NULL, "reader", "reader", "SM_DATA", 3306, NULL, CLIENT_IGNORE_SPACE | CLIENT_MULTI_STATEMENTS);

	switch (mode) {
	case 1:	//30 дней
	{
		mysql_query(hDB, "SELECT DATE_FORMAT(DateTime,\"%d.%m\"),AVG(Humidity) FROM Measures WHERE MONTH(DateTime)=MONTH(CURDATE())-1 OR MONTH(DateTime)=MONTH(CURDATE())  GROUP BY DAYOFYEAR(DateTime) ORDER BY DateTime DESC LIMIT 30;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Humidity(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount - 1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string d_s = dt.substr(0, 2);
			string M_s = dt.substr(3, 2);
			int d = atoi(d_s.c_str());
			int M = atoi(M_s.c_str());
			data[i].h_or_d = d;
			data[i].m_or_M = M;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Humidity(mode=%d): %d.%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x20;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;

	default:
	case 0:	//За последние сутки
	{
		mysql_query(hDB, "SELECT TIME_FORMAT(DateTime,\"%H:%i\"),Humidity from Measures order by DateTime desc LIMIT 96;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Humidity(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount - 1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string h_s = dt.substr(0, 2);
			string m_s = dt.substr(3, 2);
			int h = atoi(h_s.c_str());
			int m = atoi(m_s.c_str());
			data[i].h_or_d = h;
			data[i].m_or_M = m;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Humidity(mode=%d): %d:%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x20;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;
	}
}

int PackPressureData(char mode, int reciver) {
	MYSQL* hDB = mysql_init(NULL);
	hDB = mysql_real_connect(hDB, NULL, "reader", "reader", "SM_DATA", 3306, NULL, CLIENT_IGNORE_SPACE | CLIENT_MULTI_STATEMENTS);

	switch (mode) {
	case 1:	//30 дней
	{
		mysql_query(hDB, "SELECT DATE_FORMAT(DateTime,\"%d.%m\"),AVG(Pressure) FROM Measures WHERE MONTH(DateTime)=MONTH(CURDATE())-1 OR MONTH(DateTime)=MONTH(CURDATE())  GROUP BY DAYOFYEAR(DateTime) ORDER BY DateTime DESC LIMIT 30;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Pressure(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount - 1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string d_s = dt.substr(0, 2);
			string M_s = dt.substr(3, 2);
			int d = atoi(d_s.c_str());
			int M = atoi(M_s.c_str());
			data[i].h_or_d = d;
			data[i].m_or_M = M;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Pressure(mode=%d): %d.%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x30;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;

	default:
	case 0:	//За последние сутки
	{
		mysql_query(hDB, "SELECT TIME_FORMAT(DateTime,\"%H:%i\"),Pressure from Measures order by DateTime desc LIMIT 96;");
		MYSQL_RES* res = mysql_store_result(hDB);

		int rcount = res->row_count;
		printf("SQL_Pressure(mode=%d): rows count %d\n", mode, rcount);
		TimeATemp* data = new TimeATemp[rcount];
		for (int i = rcount - 1; i >= 0; i--) {
			MYSQL_ROW row = mysql_fetch_row(res);
			string dt = std::string(row[0]);
			string h_s = dt.substr(0, 2);
			string m_s = dt.substr(3, 2);
			int h = atoi(h_s.c_str());
			int m = atoi(m_s.c_str());
			data[i].h_or_d = h;
			data[i].m_or_M = m;
			data[i].temp = strtof(row[1], NULL);
			printf("SQL_Pressure(mode=%d): %d:%d %.2f\n", mode, data[i].h_or_d, data[i].m_or_M, data[i].temp);
		}

		int len = 4 + rcount * sizeof(TimeATemp);
		char* buffer = new char[len];
		buffer[0] = 0xAA;
		buffer[1] = 0x55;
		buffer[2] = 0x30;
		buffer[3] = rcount;
		memcpy(buffer + 4, data, rcount * sizeof(TimeATemp));

		write(reciver, buffer, len);
		mysql_close(hDB);
		delete[] data;
		delete[] buffer;

		return len;
	}
	break;
	}
}

void * TCPReciverThread::Recive(void * param)
{
	RTData* data = (RTData*)param;
	int reciver = data->iSocket;
	bool* stopflag = data->bStopFlag;
	char buf[512];
	int bytes_read;
	int cnt = 0;
	printf("RECV(%d): Start working\n", reciver);
	while (1) {
		//this_thread::sleep_for(std::chrono::microseconds(1));
		bytes_read = recv(reciver, buf, 512, 0);
		if (bytes_read <= 0) break;
		//проверка
		if (buf[0] != 0xAA || buf[1] != 0x55) break;
		//обработка
		switch (buf[2])
		{
		case 0x05: //Main
			cnt = GetLastMeasure(reciver);
			if(cnt>0)
				printf("RECV(%d): Send last measures to client\n", reciver);
			else
				printf("RECV(%d): GetLastMeasure error\n", reciver);
			break;
		case 0x10: //Temperature
			cnt = PackTemperatureData(buf[3], reciver);
			if (cnt > 0)
				printf("RECV(%d): Send temperature data to client\n", reciver);
			else
				printf("RECV(%d): Error at PackTemperatureData\n", reciver);
			break;
		case 0x20: //Humidity
			cnt = PackHumidityData(buf[3], reciver);
			if (cnt > 0)
				printf("RECV(%d): Send humidity data to client\n", reciver);
			else
				printf("RECV(%d): Error at PackHumidityData\n", reciver);
			break;
		case 0x30: //Pressure
			cnt = PackPressureData(buf[3], reciver);
			if (cnt > 0)
				printf("RECV(%d): Send pressure data to client\n", reciver);
			else
				printf("RECV(%d): Error at PackPressureData\n", reciver);
			break;
		default:
			break;
		}
	}
	printf("RECV(%d): Stop working\n", reciver);
	*stopflag = true;
	close(reciver);
	return 0;
}

TCPReciverThread::~TCPReciverThread()
{
	pthread_cancel(threadHandle);
}
