#include <iostream>
#include <sstream>
#include <vector>
#include <string>

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string line;

    while (std::getline(ss, line, delimiter)) {
        // std::getline()
        result.push_back(line);
    }
    return (result);
}
// int main ()
// {
//     int i = 0;
//     std::string a = "taha reda hamza to high";
//     std::vector<std::string> r = split(a, ' ');
//    for (int i = 0; i < r.size(); i++) {
//     std::cout << r[i] << std::endl;
//    }
// }
