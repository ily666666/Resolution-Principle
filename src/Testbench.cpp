#include "Testbench.h"
#include "subsentence.h"
#include "preprocessing.h"
#include "Resolution.h"
#include <iostream>
#include <fstream>
#include <iterator>
using namespace std;
using namespace FormulaNamepace;

void Test_All()
{
	Test_StrStr();
	Test_Substitution();
	Test_SubstitutionSet();
	Test_ComposeSustitutes();
	Test_MGU();
	Test_Resolution();
	Test_ResolutionSteps();
}

ostream& PrintSusentenceSet(ostream& os,
							const SubsentenceSet& subset)
{
	copy(subset.begin(), subset.end(),
			ostream_iterator<Subsentence>(os, " "));
	return os;
}

void Test_ResolutionSteps()
{
	ifstream is("formula.txt");
	ostream& os = cout;

	os << "Please enter premise and conclusion "
		  "separated by line.Notice you need "
		  "to enter the negation of the conclusion: " << endl;
	string premise, conclusion;
	getline(is, premise); getline(is, conclusion);

	os << "You entered:" << endl;
	os << "premise: " << premise << endl;
	os << "conclusion: " << conclusion << endl << endl;

	RemoveSpace(premise);
	RemoveSpace(conclusion);
	RemoveOuterBracket(premise);
	RemoveOuterBracket(conclusion);
	os << "After precessing:" << endl;
	os << "premise: " << premise << endl;
	os << "conclusion: " << conclusion << endl << endl;

	os << "After remove implication:" << endl;
	os << "premise: " << RemoveImplication(premise) << endl;
	os << "conclusion: " << RemoveImplication(conclusion)
		<< endl << endl;

	os << "After move negation to inside:" << endl;
	os << "premise: " << MoveNegation(premise) << endl;
	os << "conclusion: " << MoveNegation(conclusion)
		<< endl << endl;

	os << "After standardize values:" << endl;
	os << "premise: " << StandardizeValues(premise) << endl;
	os << "conclusion: " << StandardizeValues(conclusion)
		<< endl << endl;

	os << "After transform to prenex normal form:" << endl;
	os << "premise: " << TransformToPNF(premise) << endl;
	os << "conclusion: " << TransformToPNF(conclusion)
		<< endl << endl;

	os << "After remove existential quantifier:" << endl;
	os << "premise: " << RemoveEQ(premise) << endl;
	os << "conclusion: " << RemoveEQ(conclusion)
		<< endl << endl;

	os << "After remove  universal quantifier:" << endl;
	os << "premise: " << RemoveUQ(premise) << endl;
	os << "conclusion: " << RemoveUQ(conclusion)
		<< endl << endl;

	os << "After extract subsentence:" << endl;
	SubsentenceSet premiseSet, conclusionSet;
	ExtractSubsentence(premiseSet, premise);
	ExtractSubsentence(conclusionSet, conclusion);
	os << "subsentences of premise: ";
	PrintSusentenceSet(os, premiseSet) << endl;
	os << "subsentences of conclusion: ";
	PrintSusentenceSet(os, conclusionSet) << endl;

	try {
		SubsentenceSet concluSet;
		Resolution(premiseSet, conclusionSet, concluSet);
		os << "The given clauses are unsatisfied!" << endl;
	}catch(const ResolutionException& e) {
		os << e.what() << endl;
	}
}

void Test_StrStr()
{
	cout << "-------------------------" << endl;
	cout << "Test_FindSubStrBound:" << endl;
	string 	str = "Hello, world#@#",
			target = "He";
	cout << "Source: " << str << endl;
	cout << "Target: " << target << endl;
	auto iter = StrStr(str, target);
	string output(iter, iter + target.size());
	cout << "Result: " << output << endl;
	cout << "-------------------------" << endl;
}

ostream& PrintSustitute(ostream& os,
						const Sustitute& substi)
{
	os << substi.first << "/" << substi.second;
	return os;
}

ostream& PrintSustitutes(ostream& os,
						const Sustitutes& substiSet)
{
	for(auto iter = substiSet.begin(); iter != substiSet.end();
			++iter)
	{
		PrintSustitute(os, *iter)  << " ";
	}
	return os;
}

void Test_Substitution()
{
	cout << "-------------------------" << endl;
	cout << "Test_Substitution:" << endl;
	string 	str = "Hello, world#@#";
	Sustitute substi = make_pair("T^T", "#@#");
	cout << "Source: " << str << endl;
	cout << "Substitute: ";
	PrintSustitute(cout, substi) << endl;
	Substitution(str, substi);
	cout << "Result: " << str << endl;
	cout << "-------------------------" << endl;
}

void Test_SubstitutionSet()
{
	cout << "-------------------------" << endl;
	cout << "Test_SubstitutionSet:" << endl;
	string 	str = "Hello, world#@#";
	Sustitutes substiSet = {
								make_pair("T^T", "#@#"),
								make_pair("LOL", "llo")
							};
	cout << "Source: " << str << endl;
	cout << "Substitute: ";
	PrintSustitutes(cout, substiSet) << endl;;
	Substitution(str, substiSet);
	cout << "Result: " << str << endl;
	cout << "-------------------------" << endl;
}

void Test_ComposeSustitutes()
{
	cout << "-------------------------" << endl;
	cout << "Test_ComposeSustitutes:" << endl;
	Sustitutes substiSet1 = {
								make_pair("f(y)", "x"),
								make_pair("z", "y"),
							};
	Sustitutes substiSet2 = {
								make_pair("a", "x"),
								make_pair("b", "y"),
								make_pair("y", "z"),
							};
	cout << "Substitutes 1: ";
	PrintSustitutes(cout, substiSet1) << endl;
	cout << "Substitutes 2: ";
	PrintSustitutes(cout, substiSet2) << endl;
	Sustitutes substiSet3 =
		ComposeSustitutes(substiSet1, substiSet2);
	cout << "Result: ";
	PrintSustitutes(cout, substiSet3) << endl;
	cout << "-------------------------" << endl;
}

void Test_MGU()
{
	cout << "-------------------------" << endl;
	cout << "Test_MGU:" << endl;
	Subsentence subsent1 = "P(a, x, f(g(y)))",
				subsent2 = "P(z, f(a), f(u))";

	cout << "subsentence 1: " << subsent1 << endl;
	cout << "subsentence 2: " << subsent2 << endl;
	try {
		Sustitutes mgu = MGU(subsent1, subsent2);
		cout << "MGU: ";
		PrintSustitutes(cout, mgu) << endl;
		cout << "-------------------------" << endl;
	}catch(ResolutionException& e) {
		cerr << e.what() << endl;
	}
}

void Test_Resolution()
{
	cout << "-------------------------" << endl;
	cout << "Test_Resolution:" << endl;
	Subsentence subsent1 = "P(x)|Q(x,y)",
				subsent2 = "~P(a)|R(b,z)";
	cout << "subsentence 1: " << subsent1 << endl;
	cout << "subsentence 2: " << subsent2 << endl;
	try {
		Subsentence resoSubsent = Resolution(subsent1, subsent2);
		cout << "Resolution: " << resoSubsent << endl;
		cout << "-------------------------" << endl;
	}catch(ResolutionException& e) {
		cerr << e.what() << endl;
	}
}
