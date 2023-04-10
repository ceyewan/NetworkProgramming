#include<bits/stdc++.h>

void hello(std::string &&str) {
    std::cout << str << std::endl;
}

int main() {
    hello("hello");
}