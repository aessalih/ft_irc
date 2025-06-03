#include <vector>
#include "Server.hpp"

std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> tokens;
    std::string currentWord;
    bool inWord = false;

    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n') {
            if (inWord) {  // End of a word
                tokens.push_back(currentWord);
                currentWord.clear();
                inWord = false;
            }
        } else {
            currentWord += c;
            inWord = true;
        }
    }

    // Add the last word if it exists
    if (inWord) {
        tokens.push_back(currentWord);
    }

    return tokens;
}

#include <sstream>

int main ()
{
    int i = 10;

    // std::string tmp;
    
    std::stringstream iss;
    iss << i;
    std::string a  = "tah " + iss.str();
    std::cout << a << std::endl;

    // std::string taha = "taha reda                                                                                        ";
    // std::vector<std::string> r;
    // r = split(taha);
    // for(int i = 0; i < r.size(); i++)
    // {
    //     std::cout << "(" << r[i] << ")\n";
    // }
    // std::string a = "hello " + 10;
    // std::cout << a << std::endl;
}