#ifndef SUBSENTENCE_H
#define SUBSENTENCE_H

#include <string>
#include <set>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <cctype>

namespace FormulaNamepace {

	// 公式符号定义

	const char EQ = '#';	// 存在量词符号
	const char UQ = '@';	// 全称量词符号
	const char IMPLICATION = '>';	// 蕴含符号
	const char NEGATION = '~';		// 否定符号
	const char CONJUNCTION = '&';	// 合取符号
	const char DISJUNCTION = '|';	// 析取符号

	const char CONSTANT_ALPHA[] = { 'a', 'b', 'c', 'd', 'e',
									'i', 'j', 'k'};

	typedef std::string Formula;
	typedef std::string Subsentence;
	typedef std::set<Subsentence> SubsentenceSet;

	bool IsConstantAlpha(char ch);

	// 移除最外层的括号对
	Formula& RemoveOuterBracket(Formula& f);

	// 前向扫描，查找匹配的一对组件，并且返回后者所在位置
	template<typename FwdIter, typename Compo>
	FwdIter FindPairChar(FwdIter first, FwdIter last,
										Compo former, Compo latter)
	{
		std::size_t pairCnt = 0;
		while(1) {
			if(first == last) return last;

			if(*first == former)
				++pairCnt;
			else if(*first == latter)
				if(--pairCnt == 0)
					break;
			++first;
		}
		return first; // 现在rbegin指向匹配的左括号的位置
	}

	// 查找谓词符号
	template<typename FwdIter>
	inline FwdIter FindPredicate(FwdIter first, FwdIter last)
	{
		// 谓词是用小写字母表达的
		return std::find_if(first, last,
							std::ptr_fun<int, int>(islower));
	}

	// 查找公式符号
	template<typename FwdIter>
	inline FwdIter FindFormula(FwdIter first, FwdIter last)
	{
		// 公式是用大写字母表达的
		return std::find_if(first, last,
							std::ptr_fun<int, int>(isupper));
	}

	// 查找量词符号
	template<typename FwdIter>
	inline FwdIter FindQuantifier(FwdIter first, FwdIter last)
	{
		return std::find_if(first, last,
						[] (const typename FwdIter::value_type& lhs)
						{ return lhs == EQ || lhs == UQ; }
					);
	}

	template<typename Set>
	char FindNewLowerAlpha(const Set& s)
	{
		const size_t alphaCnt = 26;
		std::default_random_engine e;
		for(size_t i = 0; i < alphaCnt; ++i) {
			char ch = 'a' + e() % alphaCnt;
			if(s.find(ch) == s.end())
				return ch;
		}
		throw std::logic_error("no more new alpha can use");
	}

	inline bool IsConnector(char ch)
	{
		return ch == CONJUNCTION || ch == DISJUNCTION;
	}

	Formula& ReplaceAlphaWithString(Formula& target,
					char alpha, const Formula& str);

	// 获取谓词串
	Subsentence GetPredicate(
					typename Subsentence::const_iterator first,
					typename Subsentence::const_iterator last);

	// 获取公式串
	Subsentence GetFormula(
					typename Subsentence::const_iterator first,
					typename Subsentence::const_iterator last);

	// 消去蕴含连接词
	Formula& RemoveImplication(Formula& f);

	// 将否定符号移到紧靠谓词的位置
	Formula& MoveNegation(Formula& f);

	// 对变元标准化
	Formula& StandardizeValues(Formula& f);

	// 化为前束范式
	Formula& TransformToPNF(Formula& f);

	// 化为Skolem标准型
	Formula& TransformToSkolem(Formula& f);

	// 消去存在量词
	Formula& RemoveEQ(Formula& f);

	// 消去全称量词
	Formula& RemoveUQ(Formula& f);

	// 消去合取符号，获得子句集
	void ExtractSubsentence(SubsentenceSet& subset,
							const Formula& f);
}
#endif // SUBSENTENCE_H
