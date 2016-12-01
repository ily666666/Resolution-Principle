# Resolution-Principle
Implement of resolution principle, offer automatic machine reasoning, programming by C++ language.

## notes
Note that the file "*/formula.txt" save the clauses you want to prove.The content format of this file are:

- premise
- negation of conclusion

The following clauses are the current content of this file:

- (@x)(P(x)>Q(x))
- ~(@x)(~Q(x)>~P(x))
  
Actually, we are going to prove "(@x)(P(x)>Q(x)) -> (@x)(~Q(x)>~P(x))".
