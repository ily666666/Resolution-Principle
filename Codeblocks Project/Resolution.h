#ifndef RFESOLUTION_H
#define RFESOLUTION_H

#include <set>
#include <utility>
#include <string>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <exception>
#include "subsentence.h"

typedef std::pair<FormulaNamepace::Formula,
				  FormulaNamepace::Formula> Sustitute;
typedef std::set<Sustitute> Sustitutes;

// �����û��û��Ӿ䡣�����û�������true�����򷵻�false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitute& susbtitute);

// �����û����û��Ӿ䡣������ȫ�û�������true�����򷵻�false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitutes& substitutes);

// ���û��ϳɼ�
Sustitutes ComposeSustitutes(const Sustitutes& lhs,
							 const Sustitutes& rhs);

// �����Ӿ����һ���һ(mgu)�������ɺ�һ���׳�ResulutionException�쳣
Sustitutes MGU(const FormulaNamepace::Subsentence& subsent1,
				const FormulaNamepace::Subsentence& subsent2);

// �Ƴ��Ӿ��е�ָ��ν��������������ӷ�
void RemovePredicate(FormulaNamepace::Subsentence& subsent,
					 const FormulaNamepace::Subsentence& pred);

// �������Ӿ��Ĺ�ᣬ���ع���Ӿ�
FormulaNamepace::Subsentence
Resolution(const FormulaNamepace::Subsentence& subsent1,
		   const FormulaNamepace::Subsentence& subsent2);

// �������Ӿ伯֮��Ĺ�ᣬ�����ŵ�resolutionSet��
void Resolution(const FormulaNamepace::SubsentenceSet& premiseSet,
				const FormulaNamepace::SubsentenceSet& conclusionSet,
				FormulaNamepace::SubsentenceSet& resolutionSet);

// strstr�ĵ������汾
template<typename String>
typename String::iterator StrStr(String& str, const String& target)
{
	using namespace std;
	// �÷���pch��ǰ�棬��Ȼ�ڲ�������ĳʱ�ᱻ�ƶ�
	auto iter = str.begin();
	const char* const pch = strstr(str.c_str(), target.c_str());
	if(pch == nullptr)
		return str.end();
	iter = find_if(
					iter, str.end(),
					[pch] (const typename
								String::iterator::value_type& lhs)
					{ return static_cast<const char*>(&lhs) == pch; }
				);
	assert(iter != str.end());
	return iter;
}

class ResolutionException : public std::exception {
public:
	ResolutionException(const char* mesg = "invalid subsentence")
	: mesg_(mesg) { }

	const char* what() const throw()
	{
		return mesg_.c_str();
	}
private:
	std::string mesg_;
};
#endif // RFESOLUTION_H
