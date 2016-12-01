#include "Resolution.h"

using namespace std;
using namespace FormulaNamepace;

// 根据置换置换子句。若可置换，返回true，否则返回false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitute& susbtitute)
{
	bool canSubstitute = false;
	const Formula& from = susbtitute.second;
	// 查找被替换项
	for(auto substrIter = StrStr(subsent, from);
			 substrIter != subsent.end();
			 substrIter = StrStr(subsent, from)) {
		const Formula& to = susbtitute.first;
		// 执行置换 to -> from
		subsent.erase(substrIter,  substrIter + from.size());
		subsent.insert(substrIter, to.begin(), to.end());
	}
	return canSubstitute;
}

// 根据置换集置换子句。若可完全置换，返回true，否则返回false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitutes& substitutes)
{
	bool canSubstitute = true;
	// 在尝试了一早上的bind, functional, ptr_fun，历经失败后，
	// :-D 我放弃了使用for_each，如果我不重载Substitution的话···
	for(auto iter = substitutes.begin(); iter != substitutes.end();
			++iter) {
		if(!Substitution(subsent, *iter))
			canSubstitute = false;
	}
	return canSubstitute;
}

// 求置换合成集
Sustitutes ComposeSustitutes(const Sustitutes& lhs,
							 const Sustitutes& rhs)
{
	Sustitutes compSet;
	for(auto iter1 = lhs.begin(); iter1 != lhs.end(); ++iter1) {
		string substitutedItem = iter1->first;
		auto predicateIter = FindPredicate(
				substitutedItem.rbegin(), substitutedItem.rend());
		assert(predicateIter != substitutedItem.rend());

		string valueName(1, *predicateIter); // 根据被替换项找替换项
		for(auto iter2 = rhs.begin(); iter2 != rhs.end(); ++iter2) {
			if(valueName == iter2->second) {
				ReplaceAlphaWithString(substitutedItem, *predicateIter,
										iter2->first);
				break; // 置换集不含有多个同值的替换，因此只会找到一次
			}
		}
		if(substitutedItem != iter1->second) {// 不需要像x/x这种置换
			compSet.insert(make_pair(substitutedItem, iter1->second));
		}
	}

	for(auto iter2 = rhs.begin(); iter2 != rhs.end(); ++iter2) {
		bool notEqual = true;
		for(auto iter1 = lhs.begin(); iter1 != lhs.end(); ++iter1) {
			if(iter2->second == iter1->second) {
				notEqual = false;
				break;
			}
		}
		if(notEqual) { // 不是同值的替换项，需要放入结果
			compSet.insert(*iter2);
		}
	}
	return compSet;
}

// 求两子句的最一般合一(mgu)。若不可合一，抛出ResulutionException异常
Sustitutes MGU(const FormulaNamepace::Subsentence& subsent1,
				const FormulaNamepace::Subsentence& subsent2)
{
	pair<Subsentence, Subsentence> w = { subsent1, subsent2 };
	Sustitutes mgu;
	while(w.first != w.second) { // w未合一
		// 找不一致集
		auto iter1 = FindPredicate(w.first.begin(), w.first.end());
		auto iter2 = FindPredicate(w.second.begin(), w.second.end());
		while(iter1 != w.first.end() && iter2 != w.second.end()) {
			if(*iter1 != *iter2)
				break;
			iter1 = FindPredicate(iter1 + 1, w.first.end());
			iter2 = FindPredicate(iter2 + 1, w.second.end());
		}
		// 找到不一致集合
		if(iter1 != w.first.end() && iter2 != w.second.end()) {
			string item1 = GetPredicate(iter1,  w.first.end());
			string item2 = GetPredicate(iter2,  w.second.end());
			// 不允许置换有嵌套关系
			if(StrStr(item1, item2) != item1.end() ||
				StrStr(item2, item1) != item2.end()) {
					throw ResolutionException("cannot unifier");
			}
			// 只允许常量替换变量
			if(!IsConstantAlpha(*iter1))
				item1.swap(item2);
			// 更新置换，然后置换子句集
			Sustitutes sustiSet = { make_pair(item1, item2) };
			mgu = ComposeSustitutes(mgu, sustiSet);
			Substitution(w.first, mgu);
			Substitution(w.second, mgu);
		}
		else {	// 两子句不可合一
			throw ResolutionException("cannot unifier");
		}
	}
	return mgu;
}

// 移除子句中的指定谓语，包括其参与的连接符
void RemovePredicate(FormulaNamepace::Subsentence& subsent,
					 const FormulaNamepace::Subsentence& pred)
{
	// 假设子句中应该不含有多项同样的谓语，应此只处理一次
	auto predFirst = StrStr(subsent, pred);
	if(predFirst == subsent.end()) return;

	// 判断是否该谓语是否和其他谓语连接，如P(x)|Q(x)&V(x)
	if(predFirst != subsent.begin() &&
		IsConnector(*(predFirst - 1))) { // 清除前面的连接符
		predFirst = subsent.erase(predFirst - 1);
	}

	auto predLast = predFirst + pred.size();
	predLast = subsent.erase(predFirst, predLast); // 删除谓语
	if(predLast != subsent.end() && IsConnector(*predLast)) {
		subsent.erase(predLast); // 清除后面的连接符
	}
}

// 求两个子句间的归结，返回归结子句
FormulaNamepace::Subsentence
Resolution(const FormulaNamepace::Subsentence& subsent1,
		   const FormulaNamepace::Subsentence& subsent2)
{
	for(auto iter1 = FindFormula(subsent1.begin(), subsent1.end());
			 iter1 != subsent1.end();
			 iter1 = FindFormula(iter1 + 1, subsent1.end())) {
		Subsentence pred1 = GetFormula(iter1, subsent1.end());
		for(auto iter2 = FindFormula(subsent2.begin(), subsent2.end());
				 iter2 != subsent2.end();
				 iter2 = FindFormula(iter2 + 1, subsent2.end())) {
			if(*iter1 != *iter2) // 名称不一致的谓词不能归结
				continue;
			Subsentence pred2 = GetFormula(iter2, subsent2.end());
			bool negaFirst = true;
			if(pred1.front() == NEGATION) {
				pred2.insert(pred2.begin(), NEGATION);
				negaFirst = false;
			}else {
				pred1.insert(pred1.begin(), NEGATION);
			}
			if(pred1.size() != pred2.size()) // 长度不同也不能归结
				continue;
			try {
				Sustitutes mgu = MGU(pred1, pred2); // 求mgu
				if(negaFirst) // 移除插入的否定符号
					pred1.erase(pred1.begin());
				else
					pred2.erase(pred2.begin());
				// 处理左子句
				Subsentence resoLeft(subsent1);
				RemovePredicate(resoLeft, pred1);
				// 处理右子句
				Subsentence resoRight(subsent2);
				RemovePredicate(resoRight, pred2);
				// 生成新子句
				Subsentence newSubsent(resoLeft);
				if(!resoRight.empty())
					newSubsent += DISJUNCTION + resoRight;
				Substitution(newSubsent, mgu);
				return newSubsent;
			}catch(const ResolutionException& e) {
				break;	// 不可合一则取下一项
			}
		}
	}
	throw ResolutionException("cannot resolution");
}


// 求两个子句集之间的归结，结果存放到resolutionSet中
void Resolution(const FormulaNamepace::SubsentenceSet& premiseSet,
				const FormulaNamepace::SubsentenceSet& conclusionSet,
				FormulaNamepace::SubsentenceSet& resolutionSet)
{
	bool resolutionHappened = false;
	for(auto iter2 = conclusionSet.begin();
				iter2 != conclusionSet.end(); ++iter2) {
		for(auto iter1 = premiseSet.begin();
					iter1 != premiseSet.end(); ++iter1) {
			try {
				Subsentence result = Resolution(*iter1, *iter2);
				if(result.empty()) // 已经证明不可满足
					return;
				resolutionHappened = true;
				resolutionSet.insert(result); // 新元素加入支撑集
			}catch(const ResolutionException& e) {
				// 两子句不可归结，继续处理
			}
		}
	}
	if(!resolutionHappened) {
		// 所给的子句集是不能证明不可满足的
		throw ResolutionException(
			"the given subsentences can't prove"
			" its  unsatisfiability");
	}
	SubsentenceSet newPremiseSet(premiseSet.begin(),
								 premiseSet.end());
	newPremiseSet.insert(conclusionSet.begin(),
						 conclusionSet.end());
	SubsentenceSet newResolutionSet;
	Resolution(newPremiseSet, resolutionSet, newResolutionSet);
}
