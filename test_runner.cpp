#include <iostream>
#include <fstream>
#include <vector>
#include "logic.hpp"

int main() {
    // 1. input.txt から数式を読み込む
    std::ifstream ifs("input.txt");
    if (!ifs) {
        std::cerr << "Error: Cannot open input.txt" << std::endl;
        return 1;
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    // 2. トークン化実行
    std::vector<Token> tokens = tokenize(content);

    // 3. output.txt に結果を書き出す
    std::ofstream ofs("output.txt");
    if (!ofs) {
        std::cerr << "Error: Cannot open output.txt" << std::endl;
        return 1;
    }

    ofs << "--- Tokenization Result ---" << std::endl;
    for (const auto& t : tokens) {
        std::string type_name;
        switch (t.type) {
            case TokenType::Keyword:    type_name = "[Keyword   ]"; break;
            case TokenType::Identifier: type_name = "[Identifier]"; break;
            case TokenType::Number:     type_name = "[Number    ]"; break;
            case TokenType::String:     type_name = "[String    ]"; break;
            case TokenType::LParen:     type_name = "[LParen    ]"; break;
            case TokenType::RParen:     type_name = "[RParen    ]"; break;
            case TokenType::Comma:      type_name = "[Comma     ]"; break;
            case TokenType::Operator:   type_name = "[Operator  ]"; break;
            case TokenType::Unknown:    type_name = "[Unknown   ]"; break;
        }
        ofs << type_name << " : " << t.value << std::endl;
    }

    std::cout << "Test completed. Check output.txt" << std::endl;
    return 0;
}