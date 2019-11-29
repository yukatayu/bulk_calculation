#pragma once
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <cassert>

using Real = long double;
using Context = std::map<std::string, Real>;
#define Case break;case
#define Default break;default

#define DO_DEBUG 0
#if DO_DEBUG
#define DEBUG(...) __VA_ARGS__
#else
#define DEBUG(...)
#endif

enum class op {
	add, sub, mul, div, div_int, mod
};

struct Calculatable {
	virtual Real calc(const Context&) = 0;
};
using Node = std::shared_ptr<Calculatable>;

struct Term final : Calculatable{
	op operation;  // "operator" is a reserved word
	std::vector<Node> operand;  // must not be nullptr
	Real calc(const Context& ctx){
		Real op0 = operand.at(0)->calc(ctx);
		Real op1 = operand.at(1)->calc(ctx);
		switch(operation){
			Case op::add:
				DEBUG(std::cerr << " > " << op0 << " + " << op1 << "  = " << op0 + op1 << std::endl);
				return op0 + op1;
			Case op::sub:
				DEBUG(std::cerr << " > " << op0 << " - " << op1 << "  = " << op0 - op1 << std::endl);
				return op0 - op1;
			Case op::mul:
				DEBUG(std::cerr << " > " << op0 << " * " << op1 << "  = " << op0 * op1 << std::endl);
				return op0 * op1;
			Case op::div:
				DEBUG(std::cerr << " > " << op0 << " / " << op1 << "  = " << op0 / op1 << std::endl);
				return op0 / op1;
			Case op::div_int:
				DEBUG(std::cerr << " > " << op0 << " // " << op1 << "  = " << std::floor(op0 / op1)<< std::endl);
				return std::floor(op0 / op1);
			Case op::mod:
				DEBUG(std::cerr << " > " << op0 << " % " << op1 << "  = " << std::fmod<Real>(op0, op1)<< std::endl);
				return std::fmod<Real>(op0, op1);
			Default:
				throw std::domain_error("undefined operation");
		}
	};
};

struct Number final : Calculatable{
	bool isVariable;
	Real num;
	std::string name;
	Number(Real num) : isVariable(false), num(num){ };
	Number(std::string name) : isVariable(true), name(name){ };
	Real calc(const Context& ctx){
		if(isVariable){
			DEBUG(std::cerr << "   > " << name << " -> " << ctx.at(name) << std::endl);
			return ctx.at(name);
		}else{
			DEBUG(std::cerr << "   > " << num << std::endl);
			return num;
		}
	}
};

Node expr(const std::string& s, int& i, std::set<std::string>&);
Node term(const std::string& s, int& i, std::set<std::string>&);
Node factor(const std::string& s, int& i, std::set<std::string>&);
Node number(const std::string& s, int& i, std::set<std::string>&);

void expect(const std::string& s, int& i, std::string str){
	if(i + str.size() > s.size())
		throw std::domain_error("unexpected EOL");
	if(auto&& sub = s.substr(i, str.size()); sub != str)
		throw std::domain_error("expected " + str + ": got " + sub);
	i += str.size();
}

bool isNumber(char c){
	return ('a' <= c && c <= 'z')
		|| ('A' <= c && c <= 'Z')
		|| ('0' <= c && c <= '9')
		|| (c == '.')
		|| (c == '_')
		|| (c == '\'')
		|| (c == '\"');
}

inline bool isDigit(char c){
	return ('0' <= c && c <= '9') || (c == '.');
}

// <expr>   ::= <term> [ ('+'|'-') <term> ]*
Node expr(const std::string& s, int& i, std::set<std::string>& valList) {
	DEBUG(std::cerr << "expr" << std::endl);
	Node node = term(s, i, valList);
	while(i < s.size() && (s.at(i) == '+' || s.at(i) == '-')) {
		auto tmp = std::make_shared<Term>();
		tmp->operand.push_back(node);
		char operation = s.at(i); ++i;
		tmp->operand.push_back(term(s, i, valList));
		switch(operation){
			Case '+':
				tmp->operation = op::add;
			Case '-':
				tmp->operation = op::sub;
			Default:
				std::domain_error("unexpected " + std::string{operation});
		}
		node = tmp;
	}
	return node;
}

// <term>   ::= <factor> [ ('*'|'/') <factor> ]*
Node term(const std::string& s, int& i, std::set<std::string>& valList) {
	DEBUG(std::cerr << "term" << std::endl);
	Node node = factor(s, i, valList);
	while(i < s.size() && (s.at(i) == '*' || s.at(i) == '/' || s.at(i) == '%')) {
		auto tmp = std::make_shared<Term>();
		tmp->operand.push_back(node);
		char operation = s.at(i); ++i;
		bool isDoubleSlash = false;
		
		if(operation == '/' && s.at(i) == '/'){
			isDoubleSlash = true;
			++i;
		}
		tmp->operand.push_back(factor(s, i, valList));
		switch(operation){
			Case '*':
				tmp->operation = op::mul;
			Case '/':
				tmp->operation =
					isDoubleSlash
						? op::div_int
						: op::div;
			Case '%':
				tmp->operation = op::mod;
			Default:
				std::domain_error("unexpected " + std::string{operation});
		}
		node = tmp;
	}
	return node;
}

// <factor> ::= <number> | '(' <expr> ')'
Node factor(const std::string& s, int& i, std::set<std::string>& valList) {
	DEBUG(std::cerr << "factor" << std::endl);
	if (i < s.size() && isNumber(s.at(i)))
		return number(s, i, valList);

	expect(s, i, "(");
	auto ret = expr(s, i, valList);
	expect(s, i, ")");
	return ret;
}

// <number> ::= [0-9a-zA-Z_']*
Node number(const std::string& s, int& i, std::set<std::string>& valList) {
	DEBUG(std::cerr << "number" << std::endl);
	std::string str;
	while(i < s.size() && isNumber(s.at(i)))
		str.push_back(s.at(i++));
	if(std::all_of(str.begin(), str.end(), isDigit))
		return std::make_shared<Number>(std::stold(str));
	valList.insert(str);
	return std::make_shared<Number>(str);
}
