#include <iostream>
#include <sstream>
#include <vector>
#include <string>


std::vector<std::string> split(const std::string &s){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	bool inWord = false;
	std::string currentWord;

	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == ' ' || s[i] == '\t') {
			if (inWord) {
				tokens.push_back(currentWord);
				currentWord.clear();
				inWord = false;
			}
		} else {
			currentWord += s[i];
			inWord = true;
		}
	}

	if (inWord && !currentWord.empty()) {
		tokens.push_back(currentWord);
	}

	return tokens;
}
int main() {
	std::string test1 = "Hello   World              This        Is A                Test                            ";


	std::vector<std::string> result1 = split(test1);
	// std::vector<std::string> result2 = split(test2);
	// std::vector<std::string> result3 = split(test3);

	std::cout << "Test 1 results:" << std::endl;
	for (size_t i = 0; i < result1.size(); i++) {
		std::cout << "[" << result1[i] << "]" << std::endl;
	}

	// std::cout << "\nTest 2 results:" << std::endl;
	// for (size_t i = 0; i < result2.size(); i++) {
	// 	std::cout << "[" << result2[i] << "]" << std::endl;
	// }

	// std::cout << "\nTest 3 results:" << std::endl;
	// for (size_t i = 0; i < result3.size(); i++) {
	// 	std::cout << "[" << result3[i] << "]" << std::endl;
	// }

	return 0;
}
