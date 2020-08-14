//
// Created by ironzhou on 2020/8/5.
//

//#include "../../include/Utils/Csplit.h"

#include "../../include/ALL.h"

void split(const std::string& s,
           std::vector<std::string>& sv,
           const char* delim) {
    sv.clear();                                 // 1.
    char* buffer = new char[s.size() + 1];      // 2.
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);      // 3.
    char* p = std::strtok(buffer, delim);       // 4.
    do {
        sv.push_back(p);                        // 5.
    } while ((p = std::strtok(NULL, delim)));   // 6.
    delete[] buffer;
    return;
}

/*int main() {
    std::string s("abc:def::ghi");
    std::vector<std::string> sv;

    split(s, sv, ":");

    for (std::vector<std::string>::const_iterator iter = sv.begin();
         iter != sv.end();
         ++iter) {
        std::cout << *iter << std::endl;
    }

    return 0;
}*/