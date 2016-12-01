#include "Resolution.h"

using namespace std;
using namespace FormulaNamepace;

// �����û��û��Ӿ䡣�����û�������true�����򷵻�false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitute& susbtitute)
{
	bool canSubstitute = false;
	const Formula& from = susbtitute.second;
	// ���ұ��滻��
	for(auto substrIter = StrStr(subsent, from);
			 substrIter != subsent.end();
			 substrIter = StrStr(subsent, from)) {
		const Formula& to = susbtitute.first;
		// ִ���û� to -> from
		subsent.erase(substrIter,  substrIter + from.size());
		subsent.insert(substrIter, to.begin(), to.end());
	}
	return canSubstitute;
}

// �����û����û��Ӿ䡣������ȫ�û�������true�����򷵻�false
bool Substitution(FormulaNamepace::Subsentence& subsent,
					const Sustitutes& substitutes)
{
	bool canSubstitute = true;
	// �ڳ�����һ���ϵ�bind, functional, ptr_fun������ʧ�ܺ�
	// :-D �ҷ�����ʹ��for_each������Ҳ�����Substitution�Ļ�������
	for(auto iter = substitutes.begin(); iter != substitutes.end();
			++iter) {
		if(!Substitution(subsent, *iter))
			canSubstitute = false;
	}
	return canSubstitute;
}

// ���û��ϳɼ�
Sustitutes ComposeSustitutes(const Sustitutes& lhs,
							 const Sustitutes& rhs)
{
	Sustitutes compSet;
	for(auto iter1 = lhs.begin(); iter1 != lhs.end(); ++iter1) {
		string substitutedItem = iter1->first;
		auto predicateIter = FindPredicate(
				substitutedItem.rbegin(), substitutedItem.rend());
		assert(predicateIter != substitutedItem.rend());

		string valueName(1, *predicateIter); // ���ݱ��滻�����滻��
		for(auto iter2 = rhs.begin(); iter2 != rhs.end(); ++iter2) {
			if(valueName == iter2->second) {
				ReplaceAlphaWithString(substitutedItem, *predicateIter,
										iter2->first);
				break; // �û��������ж��ֵͬ���滻�����ֻ���ҵ�һ��
			}
		}
		if(substitutedItem != iter1->second) {// ����Ҫ��x/x�����û�
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
		if(notEqual) { // ����ֵͬ���滻���Ҫ������
			compSet.insert(*iter2);
		}
	}
	return compSet;
}

// �����Ӿ����һ���һ(mgu)�������ɺ�һ���׳�ResulutionException�쳣
Sustitutes MGU(const FormulaNamepace::Subsentence& subsent1,
				const FormulaNamepace::Subsentence& subsent2)
{
	pair<Subsentence, Subsentence> w = { subsent1, subsent2 };
	Sustitutes mgu;
	while(w.first != w.second) { // wδ��һ
		// �Ҳ�һ�¼�
		auto iter1 = FindPredicate(w.first.begin(), w.first.end());
		auto iter2 = FindPredicate(w.second.begin(), w.second.end());
		while(iter1 != w.first.end() && iter2 != w.second.end()) {
			if(*iter1 != *iter2)
				break;
			iter1 = FindPredicate(iter1 + 1, w.first.end());
			iter2 = FindPredicate(iter2 + 1, w.second.end());
		}
		// �ҵ���һ�¼���
		if(iter1 != w.first.end() && iter2 != w.second.end()) {
			string item1 = GetPredicate(iter1,  w.first.end());
			string item2 = GetPredicate(iter2,  w.second.end());
			// �������û���Ƕ�׹�ϵ
			if(StrStr(item1, item2) != item1.end() ||
				StrStr(item2, item1) != item2.end()) {
					throw ResolutionException("cannot unifier");
			}
			// ֻ�������滻����
			if(!IsConstantAlpha(*iter1))
				item1.swap(item2);
			// �����û���Ȼ���û��Ӿ伯
			Sustitutes sustiSet = { make_pair(item1, item2) };
			mgu = ComposeSustitutes(mgu, sustiSet);
			Substitution(w.first, mgu);
			Substitution(w.second, mgu);
		}
		else {	// ���Ӿ䲻�ɺ�һ
			throw ResolutionException("cannot unifier");
		}
	}
	return mgu;
}

// �Ƴ��Ӿ��е�ָ��ν��������������ӷ�
void RemovePredicate(FormulaNamepace::Subsentence& subsent,
					 const FormulaNamepace::Subsentence& pred)
{
	// �����Ӿ���Ӧ�ò����ж���ͬ����ν�Ӧ��ֻ����һ��
	auto predFirst = StrStr(subsent, pred);
	if(predFirst == subsent.end()) return;

	// �ж��Ƿ��ν���Ƿ������ν�����ӣ���P(x)|Q(x)&V(x)
	if(predFirst != subsent.begin() &&
		IsConnector(*(predFirst - 1))) { // ���ǰ������ӷ�
		predFirst = subsent.erase(predFirst - 1);
	}

	auto predLast = predFirst + pred.size();
	predLast = subsent.erase(predFirst, predLast); // ɾ��ν��
	if(predLast != subsent.end() && IsConnector(*predLast)) {
		subsent.erase(predLast); // �����������ӷ�
	}
}

// �������Ӿ��Ĺ�ᣬ���ع���Ӿ�
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
			if(*iter1 != *iter2) // ���Ʋ�һ�µ�ν�ʲ��ܹ��
				continue;
			Subsentence pred2 = GetFormula(iter2, subsent2.end());
			bool negaFirst = true;
			if(pred1.front() == NEGATION) {
				pred2.insert(pred2.begin(), NEGATION);
				negaFirst = false;
			}else {
				pred1.insert(pred1.begin(), NEGATION);
			}
			if(pred1.size() != pred2.size()) // ���Ȳ�ͬҲ���ܹ��
				continue;
			try {
				Sustitutes mgu = MGU(pred1, pred2); // ��mgu
				if(negaFirst) // �Ƴ�����ķ񶨷���
					pred1.erase(pred1.begin());
				else
					pred2.erase(pred2.begin());
				// �������Ӿ�
				Subsentence resoLeft(subsent1);
				RemovePredicate(resoLeft, pred1);
				// �������Ӿ�
				Subsentence resoRight(subsent2);
				RemovePredicate(resoRight, pred2);
				// �������Ӿ�
				Subsentence newSubsent(resoLeft);
				if(!resoRight.empty())
					newSubsent += DISJUNCTION + resoRight;
				Substitution(newSubsent, mgu);
				return newSubsent;
			}catch(const ResolutionException& e) {
				break;	// ���ɺ�һ��ȡ��һ��
			}
		}
	}
	throw ResolutionException("cannot resolution");
}


// �������Ӿ伯֮��Ĺ�ᣬ�����ŵ�resolutionSet��
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
				if(result.empty()) // �Ѿ�֤����������
					return;
				resolutionHappened = true;
				resolutionSet.insert(result); // ��Ԫ�ؼ���֧�ż�
			}catch(const ResolutionException& e) {
				// ���Ӿ䲻�ɹ�ᣬ��������
			}
		}
	}
	if(!resolutionHappened) {
		// �������Ӿ伯�ǲ���֤�����������
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
