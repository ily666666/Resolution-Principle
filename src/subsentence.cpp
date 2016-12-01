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
		if(islower(*iter)) { // 收集已有变量名
			alphaSet.insert(*iter);
		}
	}
	for(size_t i = 0; i != sizeof(CONSTANT_ALPHA); ++i) {
		if(alphaSet.find(CONSTANT_ALPHA[i]) == alphaSet.end())
			return CONSTANT_ALPHA[i];
	}
	throw runtime_error("no more constant alpha can get");
}

// 移除最外层的括号对
Formula& RemoveOuterBracket(Formula& f)
{
	if(!f.empty() && f.front() == '(') { // 移除多余的外层括号
		auto last = FindPairChar(f.begin(), f.end(), '(', ')');
		bool hasRemoved = false;
		if(last != f.end() && last+1 == f.end()) {
			f.erase(f.begin());
			f.erase(f.end() - 1);
			hasRemoved = true;
		}
		if(hasRemoved) // 防止恶意的多层嵌套括号
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

	// 如果左边是右括号，则表明该公式被量词修饰着，
	// 则还需要移动到量词的左括号
	while((rbegin+1) != rend && *(rbegin+1) == ')') {
		rbegin = FindPairChar(rbegin+1, rend, ')', '(');
		if(rbegin == rend)
			return rend;
	}
	// 如果还被否定修饰，则也需移动到其前面
	while((rbegin+1) != rend && *(rbegin+1) == NEGATION)
		++rbegin;
	return rbegin;
}

// 获取谓词串
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

// 获取公式串
Subsentence GetFormula(
				typename Subsentence::const_iterator first,
				typename Subsentence::const_iterator last)
{
	// 查找公式尾后
	auto formulaFirst = FindFormula(first, last);
	assert(formulaFirst != last);
	auto formulaLast = FindPairChar(formulaFirst, last, '(', ')');
	assert(formulaLast != last);
	++formulaLast;	// 转换为公式尾后
	// 查找公式头
	Subsentence::const_reverse_iterator
			rbegin(formulaFirst+1), rend(first-1);
	auto revIter = GetBeginOfFormula(rbegin, rend);
	formulaFirst = revIter.base() - 1;
	return Subsentence(formulaFirst, formulaLast);
}



// 消去蕴含连接词
Formula& RemoveImplication(Formula& f)
{
	FormulaIter iter;

	while((iter = find(f.begin(), f.end(), IMPLICATION))
					!= f.end()) {
		*iter = DISJUNCTION; // 将蕴含符号替换为析取符号

		FormulaRevIter revIter(iter);
        revIter =
			GetBeginOfFormula(revIter, f.rend()); // 查找蕴含前件
        iter = revIter.base()-1;// 反向迭代器到正向迭代器转换需要减1
		f.insert(iter, NEGATION);	// 在前件前面插入否定
	}

	return f;
}

// 将否定符号移到紧靠谓词的位置
Formula& MoveNegation(Formula& f)
{
	FormulaIter iter = find(f.begin(), f.end(), NEGATION);
	while(iter != f.end()) {
		if(*(iter+1) == '(') {// 否定不是直接修饰谓词公式，需要内移
			// 否定符号修饰着带量词的谓词公式
			if(*(iter+2) == EQ || *(iter+2) == UQ) {
				// 量词取反
				*(iter+2) == EQ ? *(iter+2) = UQ : *(iter+2) = EQ;

				string leftDonePart(f.begin(), iter+5);
				// 移除否定符号
				leftDonePart.erase(find(leftDonePart.begin(),
										leftDonePart.end(), NEGATION));

				string rightPart(iter + 5, f.end());
				// 否定内移
				rightPart.insert(rightPart.begin(), NEGATION);

				// 递归处理右部分
				MoveNegation(rightPart);

				string(leftDonePart + rightPart).swap(f);
				return f;
			}else {	// 修饰着多个公式，形如~(P(x)|Q(x))
				iter = f.insert(iter+2, NEGATION);	// 内移否定符号
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
								NEGATION));// 清除原否定符号
				return MoveNegation(f);
			}
		}else if(*(iter+1) == NEGATION) {// 两个否定，直接相消
			f.erase(iter, iter + 2);
			return MoveNegation(f);	// 重新处理
		}else {
			iter = find(iter + 1, f.end(), NEGATION);
		}
	}
	return f;
}


// 对变元标准化
Formula& StandardizeValues(Formula& f)
{
	set<char> checkedAlpha;
	FormulaIter iter = FindQuantifier(f.begin(), f.end());
	while(iter != f.end()) {
		char varName = *++iter;	// 获取变量名
		if(checkedAlpha.find(varName) == checkedAlpha.end()) {
			checkedAlpha.insert(varName);
		}else {	// 变量名冲突了，需要改名
			// 获取新名字
			char newName = FindNewLowerAlpha(checkedAlpha);
			// 查找替换右边界
			FormulaIter rightBorder = FindPairChar(
									iter + 2, f.end(), '(', ')');
			// 将冲突变量名替换为新的名字
			*iter = newName;
			replace(iter, rightBorder, varName, newName);
			iter = rightBorder;	// 移动到新的开始
		}
		iter = FindQuantifier(iter, f.end());
	}

	return f;
}


// 化为前束范式
Formula& TransformToPNF(Formula& f)
{
	FormulaIter iter = FindQuantifier(f.begin(), f.end());
	if(iter == f.end())
		return f;
	else if(iter-1 == f.begin()) { // 量词已经在最前面
		iter += 3;
		string leftPart(f.begin(), iter);
		string rightPart(iter, f.end());

		TransformToPNF(rightPart);	// 递归处理右部分
		(leftPart + rightPart).swap(f);
	}else {	// 量词在内部，需要提到前面
		string quantf(iter-1, iter+3);	// 保存量词
		f.erase(iter-1, iter+3);	// 移除量词
		f.insert(f.begin(), quantf.begin(), quantf.end());
		return TransformToPNF(f);	// 继续处理
	}

	return f;
}

// 消去存在量词
Formula& RemoveEQ(Formula& f)
{
	set<char> checkedAlpha;

	FormulaIter eqIter = find(f.begin(), f.end(), EQ);
	if(eqIter == f.end()) return f;

	FormulaRevIter uqIter = find(FormulaRevIter(eqIter), f.rend(), UQ);
	if(uqIter == f.rend()) { // 该存在量词前没有任意量词
		char varName = *(eqIter + 1);
		char newName = GetNewConstantAlha(f);
		auto rightBound = FindPairChar(eqIter + 3, f.end(), '(', ')');
		assert(rightBound != f.end());
		replace(eqIter + 3, rightBound, varName, newName); // 常量化
		f.erase(eqIter - 1, eqIter + 3);	// 移除存在量词
	}else {
		// 记录公式中已经存在的字母
		copy_if(f.begin(), f.end(),
			inserter(checkedAlpha, checkedAlpha.begin()),
			ptr_fun<int, int>(isalpha));
		const char oldName = *(eqIter+1);
		// 准备任意量词的函数来替换该存在量词
		const char funcName = FindNewLowerAlpha(checkedAlpha);
		string funcFormula;
		funcFormula = funcFormula + funcName
						+ '(' + *(uqIter-1) + ')';

		f.erase(eqIter - 1, eqIter + 3);	// 移除存在量词
		ReplaceAlphaWithString(f, oldName, funcFormula);
	}
	RemoveOuterBracket(f);
	return RemoveEQ(f);	// 递归处理
}


// 消去全称量词
Formula& RemoveUQ(Formula& f)
{
	FormulaIter uqIter = find(f.begin(), f.end(), UQ);
	while(uqIter != f.end()) {
		uqIter = f.erase(uqIter-1, uqIter+3); // 直接移除全称量词
		uqIter = find(uqIter, f.end(), UQ); // 继续扫描
	}
	RemoveOuterBracket(f);
	return f;
}

// 化为Skolem标准型
Formula& TransformToSkolem(Formula& f)
{
	RemoveEQ(f);
	RemoveUQ(f);
	return f;
}

// 消去合取符号，获得子句集
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
