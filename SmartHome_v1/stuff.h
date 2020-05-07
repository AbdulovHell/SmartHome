#ifndef _STUFF_H_
#define _STUFF_H_

namespace Stuff {
	
	using namespace std;

	//проверка контрольной суммы
	bool Verify(char* buf);
	//подсчет контрольной суммы
	void CalcSum(char* buf, size_t size);
	//генерация рандомных чисел
	int RangeRand(int min, int max);

	std::string ToStr(short num);

	std::string ToStr(int num);

	std::string ToStr(uint32_t num);

	std::string ToStr(long num);

	std::string ToStr(uint64_t num);

	std::string ToStr(int64_t num);

	std::string ToStr(double num);

	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
#endif	// _STUFF_H_
