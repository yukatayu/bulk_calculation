#include <string>
#include <iostream>
#include <iomanip>
#include <set>
#include "parse.hpp"

int main(){
	std::set<std::string> valList;
	std::string equation;
	std::cout.setf(std::ios::fixed, std::ios::floatfield);
	std::cout << "input equation > ";
	std::cin >> equation;
	int i = 0;
	auto parsed = expr(equation, i, valList);
	if(valList.empty()){
		std::cout << " >> " << parsed->calc({}) << std::endl;
		return 0;
	}
	for(;;){
		std::cout << "-----------" << std::endl << std::endl;
		Context ctx;
		for(auto&& valName : valList){
			std::cout << valName << " = ";
			Real val;
			std::cin >> val;
			ctx[valName] = val;
		}
		std::cout << std::endl;
		std::cout << " >> " << parsed->calc(ctx) << std::endl;
	}
}
