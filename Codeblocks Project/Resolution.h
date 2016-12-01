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

// 根据置换置换子句。若可置换，返回true，否则返回false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitute& susbtitute);

// 根据置换集置换子句。若可完全置换，返回true，否则返回false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitutes& substitutes);

// 求置换合成集
Sustitutes ComposeSustitutes(const Sustitutes& lhs,
							 const Sustitutes& rhs);

// 求两子句的最一般合一(mgu)。若不可合一，抛出ResulutionException异常
Sustitutes MGU(const FormulaNamepace::Subsentence& subsent1,
				const FormulaNamepace::Subsentence& subsent2);

// 移除子句中的指定谓语，包括其参与的连接符
void RemovePredicate(FormulaNamepace::Subsentence& subsent,
					 const FormulaNamepace::Subsentence& pred);

// 求两个子句间的归结，返回归结子句
FormulaNamepace::Subsentence
Resolution(const FormulaNamepace::Subsentence& subsent1,
		   const FormulaNamepace::Subsentence& subsent2);

// 求两个子句集之间的归结，结果存放到resolutionSet中
void Resolution(const FormulaNamepace::SubsentenceSet& premiseSet,
				const FormulaNamepace::SubsentenceSet& conclusionSet,
				FormulaNamepace::SubsentenceSet& resolutionSet);

// strstr的迭代器版本
template<typename String>
typename String::iterator StrStr(String& str, const String& target)
{
	using namespace std;
	// 得放在pch的前面，不然内部缓冲区某时会被移动
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
