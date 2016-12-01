#include "preprocessing.h"
#include <iterator>
#include <functional>
#include <algorithm>
using namespace std;

struct IsNotSpace {
	bool operator()(char ch)
	{
		return !isspace(ch);
	}
};

// 移除空白字符
void RemoveSpace(std::string& s)
{
	string tmp;
	tmp.reserve(s.size());
	copy_if(s.begin(), s.end(), back_inserter(tmp),
				IsNotSpace());
	s.swap(tmp);
}
