#ifndef SUBSENTENCE_H
#define SUBSENTENCE_H

#include <string>
#include <set>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <cctype>

namespace FormulaNamepace {

	// ��ʽ���Ŷ���

	const char EQ = '#';	// �������ʷ���
	const char UQ = '@';	// ȫ�����ʷ���
	const char IMPLICATION = '>';	// �̺�����
	const char NEGATION = '~';		// �񶨷���
	const char CONJUNCTION = '&';	// ��ȡ����
	const char DISJUNCTION = '|';	// ��ȡ����

	const char CONSTANT_ALPHA[] = { 'a', 'b', 'c', 'd', 'e',
									'i', 'j', 'k'};

	typedef std::string Formula;
	typedef std::string Subsentence;
	typedef std::set<Subsentence> SubsentenceSet;

	bool IsConstantAlpha(char ch);

	// �Ƴ����������Ŷ�
	Formula& RemoveOuterBracket(Formula& f);

	// ǰ��ɨ�裬����ƥ���һ����������ҷ��غ�������λ��
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
		return first; // ����rbeginָ��ƥ��������ŵ�λ��
	}

	// ����ν�ʷ���
	template<typename FwdIter>
	inline FwdIter FindPredicate(FwdIter first, FwdIter last)
	{
		// ν������Сд��ĸ����
		return std::find_if(first, last,
							std::ptr_fun<int, int>(islower));
	}

	// ���ҹ�ʽ����
	template<typename FwdIter>
	inline FwdIter FindFormula(FwdIter first, FwdIter last)
	{
		// ��ʽ���ô�д��ĸ����
		return std::find_if(first, last,
							std::ptr_fun<int, int>(isupper));
	}

	// �������ʷ���
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

	// ��ȡν�ʴ�
	Subsentence GetPredicate(
					typename Subsentence::const_iterator first,
					typename Subsentence::const_iterator last);

	// ��ȡ��ʽ��
	Subsentence GetFormula(
					typename Subsentence::const_iterator first,
					typename Subsentence::const_iterator last);

	// ��ȥ�̺����Ӵ�
	Formula& RemoveImplication(Formula& f);

	// ���񶨷����Ƶ�����ν�ʵ�λ��
	Formula& MoveNegation(Formula& f);

	// �Ա�Ԫ��׼��
	Formula& StandardizeValues(Formula& f);

	// ��Ϊǰ����ʽ
	Formula& TransformToPNF(Formula& f);

	// ��ΪSkolem��׼��
	Formula& TransformToSkolem(Formula& f);

	// ��ȥ��������
	Formula& RemoveEQ(Formula& f);

	// ��ȥȫ������
	Formula& RemoveUQ(Formula& f);

	// ��ȥ��ȡ���ţ�����Ӿ伯
	void ExtractSubsentence(SubsentenceSet& subset,
							const Formula& f);
}
#endif // SUBSENTENCE_H
