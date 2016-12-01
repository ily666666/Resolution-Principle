#include "subsentence.h"
#include <cassert>
using namespace std;
using namespace FormulaNamepace;

typedef Formula::iterator FormulaIter;
typedef Formula::reverse_iterator FormulaRevIter;

namespace FormulaNamepace {

bool IsConstantAlpha(char ch)
{
	for(const char* pch = CONSTANT_ALPHA;
		pch != CONSTANT_ALPHA + sizeof(CONSTANT_ALPHA);
		++pch) {
		if(*pch == ch)
			return true;
	}
	return false;
}

char GetNewConstantAlha(const Formula& f)
{
	set<char> alphaSet;
	for(auto iter = f.begin(); iter != f.end(); ++iter) {
		if(islower(*iter)) { // �ռ����б�����
			alphaSet.insert(*iter);
		}
	}
	for(size_t i = 0; i != sizeof(CONSTANT_ALPHA); ++i) {
		if(alphaSet.find(CONSTANT_ALPHA[i]) == alphaSet.end())
			return CONSTANT_ALPHA[i];
	}
	throw runtime_error("no more constant alpha can get");
}

// �Ƴ����������Ŷ�
Formula& RemoveOuterBracket(Formula& f)
{
	if(!f.empty() && f.front() == '(') { // �Ƴ�������������
		auto last = FindPairChar(f.begin(), f.end(), '(', ')');
		bool hasRemoved = false;
		if(last != f.end() && last+1 == f.end()) {
			f.erase(f.begin());
			f.erase(f.end() - 1);
			hasRemoved = true;
		}
		if(hasRemoved) // ��ֹ����Ķ��Ƕ������
			return RemoveOuterBracket(f);
	}
	return f;
}

Formula& ReplaceAlphaWithString(Formula& target,
					char alpha, const Formula& str)
{
	FormulaIter alphaIter;
	while((alphaIter = find(target.begin(), target.end(), alpha))
							!= target.end())
	{
		alphaIter = target.erase(alphaIter);
		target.insert(alphaIter, str.begin(), str.end());
	}
	return target;
}

template<typename FormulaRevIter>
FormulaRevIter GetBeginOfFormula(FormulaRevIter rbegin,
									FormulaRevIter rend)
{
	rbegin = FindFormula(rbegin, rend);
	if(rbegin == rend) return rend;

	// �������������ţ�������ù�ʽ�����������ţ�
	// ����Ҫ�ƶ������ʵ�������
	while((rbegin+1) != rend && *(rbegin+1) == ')') {
		rbegin = FindPairChar(rbegin+1, rend, ')', '(');
		if(rbegin == rend)
			return rend;
	}
	// ������������Σ���Ҳ���ƶ�����ǰ��
	while((rbegin+1) != rend && *(rbegin+1) == NEGATION)
		++rbegin;
	return rbegin;
}

// ��ȡν�ʴ�
Subsentence GetPredicate(
				 typename Subsentence::const_iterator first,
				 typename Subsentence::const_iterator last)
{
	auto iter = FindPredicate(first, last);
	assert(iter != last);
	if(iter+1 == last || *(iter+1) != '(')
		return string(1, *iter);
	return string(iter, FindPairChar(iter, last, '(', ')') + 1);
}

// ��ȡ��ʽ��
Subsentence GetFormula(
				typename Subsentence::const_iterator first,
				typename Subsentence::const_iterator last)
{
	// ���ҹ�ʽβ��
	auto formulaFirst = FindFormula(first, last);
	assert(formulaFirst != last);
	auto formulaLast = FindPairChar(formulaFirst, last, '(', ')');
	assert(formulaLast != last);
	++formulaLast;	// ת��Ϊ��ʽβ��
	// ���ҹ�ʽͷ
	Subsentence::const_reverse_iterator
			rbegin(formulaFirst+1), rend(first-1);
	auto revIter = GetBeginOfFormula(rbegin, rend);
	formulaFirst = revIter.base() - 1;
	return Subsentence(formulaFirst, formulaLast);
}



// ��ȥ�̺����Ӵ�
Formula& RemoveImplication(Formula& f)
{
	FormulaIter iter;

	while((iter = find(f.begin(), f.end(), IMPLICATION))
					!= f.end()) {
		*iter = DISJUNCTION; // ���̺������滻Ϊ��ȡ����

		FormulaRevIter revIter(iter);
        revIter =
			GetBeginOfFormula(revIter, f.rend()); // �����̺�ǰ��
        iter = revIter.base()-1;// ��������������������ת����Ҫ��1
		f.insert(iter, NEGATION);	// ��ǰ��ǰ������
	}

	return f;
}

// ���񶨷����Ƶ�����ν�ʵ�λ��
Formula& MoveNegation(Formula& f)
{
	FormulaIter iter = find(f.begin(), f.end(), NEGATION);
	while(iter != f.end()) {
		if(*(iter+1) == '(') {// �񶨲���ֱ������ν�ʹ�ʽ����Ҫ����
			// �񶨷��������Ŵ����ʵ�ν�ʹ�ʽ
			if(*(iter+2) == EQ || *(iter+2) == UQ) {
				// ����ȡ��
				*(iter+2) == EQ ? *(iter+2) = UQ : *(iter+2) = EQ;

				string leftDonePart(f.begin(), iter+5);
				// �Ƴ��񶨷���
				leftDonePart.erase(find(leftDonePart.begin(),
										leftDonePart.end(), NEGATION));

				string rightPart(iter + 5, f.end());
				// ������
				rightPart.insert(rightPart.begin(), NEGATION);

				// �ݹ鴦���Ҳ���
				MoveNegation(rightPart);

				string(leftDonePart + rightPart).swap(f);
				return f;
			}else {	// �����Ŷ����ʽ������~(P(x)|Q(x))
				iter = f.insert(iter+2, NEGATION);	// ���Ʒ񶨷���
				while(1) {
					iter = FindFormula(iter, f.end());
					assert(iter != f.end() && "No Predicate Formula!");
					FormulaIter iter2 = FindPairChar(
										iter, f.end(), '(', ')');
					++iter2;
					if(IsConnector(*iter2)) {
						*iter2 == DISJUNCTION ? *iter2 = CONJUNCTION
											  : *iter2 = DISJUNCTION;
						iter = f.insert(iter2+1, NEGATION);
					}else
						break;
				}
				f.erase(find(f.begin(), f.end(),
								NEGATION));// ���ԭ�񶨷���
				return MoveNegation(f);
			}
		}else if(*(iter+1) == NEGATION) {// �����񶨣�ֱ������
			f.erase(iter, iter + 2);
			return MoveNegation(f);	// ���´���
		}else {
			iter = find(iter + 1, f.end(), NEGATION);
		}
	}
	return f;
}


// �Ա�Ԫ��׼��
Formula& StandardizeValues(Formula& f)
{
	set<char> checkedAlpha;
	FormulaIter iter = FindQuantifier(f.begin(), f.end());
	while(iter != f.end()) {
		char varName = *++iter;	// ��ȡ������
		if(checkedAlpha.find(varName) == checkedAlpha.end()) {
			checkedAlpha.insert(varName);
		}else {	// ��������ͻ�ˣ���Ҫ����
			// ��ȡ������
			char newName = FindNewLowerAlpha(checkedAlpha);
			// �����滻�ұ߽�
			FormulaIter rightBorder = FindPairChar(
									iter + 2, f.end(), '(', ')');
			// ����ͻ�������滻Ϊ�µ�����
			*iter = newName;
			replace(iter, rightBorder, varName, newName);
			iter = rightBorder;	// �ƶ����µĿ�ʼ
		}
		iter = FindQuantifier(iter, f.end());
	}

	return f;
}


// ��Ϊǰ����ʽ
Formula& TransformToPNF(Formula& f)
{
	FormulaIter iter = FindQuantifier(f.begin(), f.end());
	if(iter == f.end())
		return f;
	else if(iter-1 == f.begin()) { // �����Ѿ�����ǰ��
		iter += 3;
		string leftPart(f.begin(), iter);
		string rightPart(iter, f.end());

		TransformToPNF(rightPart);	// �ݹ鴦���Ҳ���
		(leftPart + rightPart).swap(f);
	}else {	// �������ڲ�����Ҫ�ᵽǰ��
		string quantf(iter-1, iter+3);	// ��������
		f.erase(iter-1, iter+3);	// �Ƴ�����
		f.insert(f.begin(), quantf.begin(), quantf.end());
		return TransformToPNF(f);	// ��������
	}

	return f;
}

// ��ȥ��������
Formula& RemoveEQ(Formula& f)
{
	set<char> checkedAlpha;

	FormulaIter eqIter = find(f.begin(), f.end(), EQ);
	if(eqIter == f.end()) return f;

	FormulaRevIter uqIter = find(FormulaRevIter(eqIter), f.rend(), UQ);
	if(uqIter == f.rend()) { // �ô�������ǰû����������
		char varName = *(eqIter + 1);
		char newName = GetNewConstantAlha(f);
		auto rightBound = FindPairChar(eqIter + 3, f.end(), '(', ')');
		assert(rightBound != f.end());
		replace(eqIter + 3, rightBound, varName, newName); // ������
		f.erase(eqIter - 1, eqIter + 3);	// �Ƴ���������
	}else {
		// ��¼��ʽ���Ѿ����ڵ���ĸ
		copy_if(f.begin(), f.end(),
			inserter(checkedAlpha, checkedAlpha.begin()),
			ptr_fun<int, int>(isalpha));
		const char oldName = *(eqIter+1);
		// ׼���������ʵĺ������滻�ô�������
		const char funcName = FindNewLowerAlpha(checkedAlpha);
		string funcFormula;
		funcFormula = funcFormula + funcName
						+ '(' + *(uqIter-1) + ')';

		f.erase(eqIter - 1, eqIter + 3);	// �Ƴ���������
		ReplaceAlphaWithString(f, oldName, funcFormula);
	}
	RemoveOuterBracket(f);
	return RemoveEQ(f);	// �ݹ鴦��
}


// ��ȥȫ������
Formula& RemoveUQ(Formula& f)
{
	FormulaIter uqIter = find(f.begin(), f.end(), UQ);
	while(uqIter != f.end()) {
		uqIter = f.erase(uqIter-1, uqIter+3); // ֱ���Ƴ�ȫ������
		uqIter = find(uqIter, f.end(), UQ); // ����ɨ��
	}
	RemoveOuterBracket(f);
	return f;
}

// ��ΪSkolem��׼��
Formula& TransformToSkolem(Formula& f)
{
	RemoveEQ(f);
	RemoveUQ(f);
	return f;
}

// ��ȥ��ȡ���ţ�����Ӿ伯
void ExtractSubsentence(SubsentenceSet& subset,
						const Formula& f)
{
	auto leftIter = f.begin(),
			middleIter = find(f.begin(), f.end(), CONJUNCTION);
	while(middleIter != f.end()) {
		subset.insert(string(leftIter, middleIter));
		leftIter = middleIter + 1;
		middleIter = find(middleIter + 1, f.end(), CONJUNCTION);
	}
	subset.insert(string(leftIter, middleIter));
}

}
