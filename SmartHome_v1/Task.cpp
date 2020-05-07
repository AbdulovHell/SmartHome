#include "sys_headers.h"

#include "Task.h"
#include "Colorize.h"
#include "main.h"
#include "I2C.h"
#include "wiringPi.h"

using namespace std;

IO::I2C* b3_8 = nullptr;
IO::I2C* b8_15 = nullptr;
IO::I2C* b15_30 = nullptr;
IO::I2C* b_bfg = nullptr;
IO::I2C* b_bfg23 = nullptr;

void Threading::TaskQuit::Run()
{
	InitShutdown();
}

void Threading::TaskRequestTemp::Run()
{
	IO::I2C dev(0x10);
	if (!dev.IsOpen())
		cout << dev.GetError() << endl;
	else {
		short data = dev.Read2BytesFromReg(2);
		//cout << "Raw temp num: " << data << endl;
		*TempStorage = data / 2.0;
	}
}

void Threading::TaskSetFreq::Run()
{
	//I2C
	if (b3_8 == nullptr) b3_8 = new IO::I2C(0x24);
	if (b8_15 == nullptr) b8_15 = new IO::I2C(0x25);
	if (b15_30 == nullptr) b15_30 = new IO::I2C(0x28);
	if (b_bfg == nullptr) b_bfg = new IO::I2C(0x26);
	if (b_bfg23 == nullptr) b_bfg23 = new IO::I2C(0x27);

	uint8_t ch = 0;
	const uint8_t cmd = 1;
	if (Freq < 4500) ch = 1;
	else if (Freq < 6000) ch = 2;
	else if (Freq < 8000) ch = 3;
	else if (Freq < 12000) ch = 4;
	else if (Freq < 15000) ch = 5;
	else if (Freq < 18000) ch = 6;
	else if (Freq < 21000) ch = 7;
	else if (Freq < 24000) ch = 8;
	else if (Freq < 26500) ch = 9;
	else if (Freq < 30000) ch = 10;
	else if (Freq < 33000) ch = 11;
	else if (Freq < 36000) ch = 12;
	else if (Freq < 40000) ch = 13;

	/*float freqs[] = {3000,6000,8000,9000,10000,12000,15000,16000,18000,19000,20000,22000,24000,25000,26000,27000,29000,30000};
	float ans[18] = { 0, };
	for (int i = 0; i < 18; i++) 
		ans[i] = ToFsint(freqs[i]);*/

	b15_30->WriteReg(ch, 1);
	b15_30->WriteReg(cmd, 0);

	b8_15->WriteReg(ch, 1);
	b8_15->WriteReg(cmd, 0);

	b3_8->WriteReg(ch, 1);
	b3_8->WriteReg(cmd, 0);

	int freq = Freq;
	printf("%s: BFG freq %d\n", Stuff::MakeColor("MAIN", Stuff::Green).c_str(), freq);
	uint8_t num[4] = { 0, };
	memcpy(num, &freq, 4);
	printf("%s: [%2X] [%2X] [%2X] [%2X]\n", Stuff::MakeColor("MAIN", Stuff::Green).c_str(), num[0], num[1], num[2], num[3]);
	//bfg
	for (int i = 0; i < 4; i++) b_bfg->WriteReg(num[i], 4 + i);
	b_bfg->WriteReg(cmd, 0);
	//bfg23
	for (int i = 0; i < 4; i++) b_bfg23->WriteReg(num[i], 4 + i);
	b_bfg23->WriteReg(cmd, 0);

	printf("%s: I2C OK freq\n", Stuff::MakeColor("MAIN", Stuff::Green).c_str());
	

	printf("%s: Set channel: %d ; Freq: %d MHz\n", Stuff::MakeColor("MAIN", Stuff::Green).c_str(), ch, Freq);
	//Stuff::Storage->SetFreq(Freq);
}

void Threading::TaskAdjustBL::Run()
{
	IO::I2C dev(0x10);
	if (!dev.IsOpen())
		cout << dev.GetError() << endl;
	else {
		uint8_t param = (127 + Step * 8);
		printf("New BL value: %d\n", param);
		dev.WriteReg(param, 1);
		dev.WriteReg((uint8_t)1, 0);
	}
	//Stuff::Storage->SetBrightLvl(Step);
}

void Threading::TaskSetPWM::Run()
{
	IO::I2C dev(0x10);
	if (!dev.IsOpen())
		cout << dev.GetError() << endl;
	else {
		uint8_t param = Step * 10;
		printf("Set pwm: %d%\n", param);
		dev.WriteReg(param, 4);
		dev.WriteReg((uint8_t)1, 0);
	}
}

void Threading::TaskSetOutput::Run()
{
	if (b3_8 == nullptr) b3_8 = new IO::I2C(0x24);
	if (b_bfg23 == nullptr) b_bfg23 = new IO::I2C(0x27);

	b3_8->WriteReg(code, 3);
	b3_8->WriteReg((uint8_t)1, 0);

	b_bfg23->WriteReg(code, 3);
	b_bfg23->WriteReg((uint8_t)1, 0);

	//Stuff::Storage->SetIF(code);
}

void Threading::TaskSetAtt::Run()
{
	const uint8_t cmd = 1;
	//I2C
	if (b3_8 == nullptr) b3_8 = new IO::I2C(0x24);
	if (b8_15 == nullptr) b8_15 = new IO::I2C(0x25);
	if (b15_30 == nullptr) b15_30 = new IO::I2C(0x28);
	//if (b_bfg == nullptr) b_bfg = new IO::I2C(0x26);
	//15-30
	b15_30->WriteReg(RFatt, 2);
	b15_30->WriteReg(cmd, 0);
	printf("Set att 15-30: %d\n", RFatt);
	//8-15
	b8_15->WriteReg(RFatt, 2);
	b8_15->WriteReg(cmd, 0);
	printf("Set att 8-15: %d\n", RFatt);
	//3-8
	if (RFatt > 15) RFatt = 15;
	uint8_t temp = (RFatt << 4) + (IFatt & 0xF);
	printf("Set att 3-8: 0x%X\n", temp);
	b3_8->WriteReg(temp, 2);
	b3_8->WriteReg(cmd, 0);

	//Stuff::Storage->SetAtt(RFatt, IFatt);
}