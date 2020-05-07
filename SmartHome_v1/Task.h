#ifndef _TASK_H_
#define _TASK_H_

namespace Threading{

	enum TaskType : int {
		SetAttCh,
		SetFreq,
		Quit,
		RequestTemp,
		AdjustBL,
		SetPWM,
		SetOutput,
		SetAtt,
		ChangeRef,
		Empty
	};

	//Родитель
	class Task {
	public:
		Task() {}
		virtual TaskType GetType() = 0;
		virtual void Run() = 0;
	};

	//Потомки
	
	class TaskSetAtt : virtual public Task {
		uint8_t RFatt;
		uint8_t IFatt;
	public:
		TaskSetAtt(uint8_t _RFatt, uint8_t _IFatt) { RFatt = _RFatt; IFatt = _IFatt; }

		TaskType GetType() { return TaskType::SetAtt; }
		void Run();
	};

	class TaskSetFreq : virtual public Task {
		uint32_t Freq=0;
	public:
		TaskSetFreq(uint32_t _freq) { 
			Freq = _freq;
			//printf("new task freq %d \n",Freq);
		}

		TaskType GetType() { return TaskType::SetFreq; }
		void Run();
	};

	class TaskAdjustBL : virtual public Task {
		uint8_t Step = 0;
	public:
		TaskAdjustBL(uint8_t _step) {
			Step = _step;
			//printf("new task freq %d \n", Freq);
		}

		TaskType GetType() { return TaskType::AdjustBL; }
		void Run();
	};

	class TaskSetPWM : virtual public Task {
		uint8_t Step = 0;
	public:
		TaskSetPWM(uint8_t _step) {
			Step = _step;
			//printf("new task freq %d \n", Freq);
		}

		TaskType GetType() { return TaskType::SetPWM; }
		void Run();
	};

	class TaskSetOutput : virtual public Task {
		uint8_t code = 0;
	public:
		TaskSetOutput(uint8_t _code) {
			code = _code;
			//printf("new task freq %d \n", Freq);
		}

		TaskType GetType() { return TaskType::SetOutput; }
		void Run();
	};
	
	class TaskQuit : virtual public Task {
	public:
		TaskQuit() {}

		TaskType GetType() { return TaskType::Quit; }
		void Run();
	};

	class TaskRequestTemp : virtual public Task {
		float* TempStorage;
	public:
		TaskRequestTemp(float* temp_s) { TempStorage = temp_s; }

		TaskType GetType() { return TaskType::RequestTemp; }
		void Run();
	};

}

#endif