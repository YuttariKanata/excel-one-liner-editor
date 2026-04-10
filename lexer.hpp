#pragma once
#include <cctype>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <iomanip> // 表示を整えるため
#include <string>


enum class TokenType { Identifier, Number, String, Operator, LParen, RParen, LBrace, RBrace, LBracket, RBracket, Semicolon, Comma, Comparison, Error, Unknown, Exclamation };

struct Token {
    TokenType type;
    std::string value;
};


inline std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    tokens.reserve(static_cast<int>(input.size()*0.6));  // 経験則
    size_t i = 0;
    while (i < input.length()) {
        char c = input[i];
        // (space) は使われないという判断
        if (isspace(c)) { i++; continue; }

        // 識別子(変数,関数)    一番多い
        if (isalpha(c) || c == '_' || c == '$') {
            size_t start = i;
            i++;
            while(i < input.length() && (isalpha(input[i]) || input[i] == '_' || input[i] == '$' || isdigit(input[i]))){
                i++;
            }
            tokens.push_back({TokenType::Identifier, std::string(&input[start], i - start)});
            continue;
        }

        bool switch_flag = true;
        switch (c)
        {
            case '(': tokens.push_back({ TokenType::LParen,     "(" }); break;
            case ')': tokens.push_back({ TokenType::RParen,     ")" }); break;
            case '{': tokens.push_back({ TokenType::LBrace,     "{" }); break;
            case '}': tokens.push_back({ TokenType::RBrace,     "}" }); break;
            case '[': tokens.push_back({ TokenType::LBracket,   "[" }); break;
            case ']': tokens.push_back({ TokenType::RBracket,   "]" }); break;
            case ';': tokens.push_back({ TokenType::Semicolon,  ";" }); break;
            case ',': tokens.push_back({ TokenType::Comma,      "," }); break;
            case '!': tokens.push_back({ TokenType::Exclamation,"!" }); break;
            case '+': case '-': case '*': case '/': case '%': case '^':
                tokens.push_back({TokenType::Operator, {c}});
                // char c だが {c} でstringにするというテク 
            break;
            case '<':
                if (i+1 < input.length() && (input[i+1] == '>' || input[i+1] == '=')) {
                    i++;
                    tokens.push_back({TokenType::Comparison, {c, input[i]}});
                }else{
                    tokens.push_back({TokenType::Comparison, "<"});
                }
            break;
            case '>':
                if (i+1 < input.length() && input[i+1] == '=') {
                    i++;
                    tokens.push_back({TokenType::Comparison, ">="});
                }else{
                    tokens.push_back({TokenType::Comparison, ">"});
                }
            break;
            case '=':
                tokens.push_back({TokenType::Comparison, "="});
            break;
            default:
                switch_flag = false;
            break;
        }
        if (switch_flag) {
            i++;
            continue;
        }
        
        // 数字
        if (isdigit(c) || c == '.') {
            size_t start = i;
            i++;
            bool exp_flag = false;
            bool dot_flag = (c == '.');
            bool error_flag = false;
            bool break_flag = false;

            while (true) {
                if (i >= input.length()) {
                    break_flag = true;
                    break;
                }

                if (isdigit(input[i])) {
                }else{switch (input[i]) {
                    case 'e':
                    case 'E':
                        if (exp_flag) {
                            // すでにeかEがある
                            error_flag = true;
                        }else{
                            exp_flag = true;
                        }
                    break;
                    case '.':
                        if (dot_flag || exp_flag) {
                            // すでに.が打ってある or すでにeかEが書かれている のにまた.が打ってある->エラー
                            error_flag = true;
                        }else{
                            dot_flag = true;
                        }
                    break;
                    case '+':
                    case '-':
                        if (!(input[i-1] == 'e' || input[i-1] == 'E')) {
                            // 2文字目以降を捜査しているのに-,+が出てきた->一つ後ろがeかEでないといけない
                            error_flag = true;
                        }else {
                        }
                    break;
                    default:
                        if (isalpha(input[i]) || input[i] == '_') {
                            // 数字の後ろの文字は , ) : - + * / % ^ など記号のみではなかろうか
                            error_flag = true;
                            i++;
                            while(i < input.length() && (isalpha(input[i]) || isdigit(input[i]) || input[i] == '_')){
                                i++;
                            }
                        }else {
                            break_flag = true;
                        }
                    break;
                }}

                if (error_flag || break_flag) {
                    break;
                }else{
                    i++;
                    continue;
                }
            }
            if (!isdigit(input[i-1])) {
                if (!exp_flag && input[i-1] == '.')
                {
                    // 必ず数字ではなく、最後が小数点のときもある。
                }else{
                    // idの最後は数字
                    error_flag = true;
                }
            }
            if (dot_flag && i - start == 1) {
                // 「.」の一文字は数字ではない
                error_flag = true;
            }

            if (error_flag) {
                tokens.push_back({TokenType::Error, std::string(&input[start], i - start)});    // i++されていて、植木算が起きない
            }else if (break_flag) {
                tokens.push_back({TokenType::Number, std::string(&input[start], i - start)});
            }
            continue;
        }

        // 文字列
        if (c == '"') {
            size_t start = i;
            i++;
            while (i+1 < input.length()) {  // i+1 == input.length() でbreakする。つまりはi=input.length()-1なのでinputの最後を指している
                if (input[i] != '"') {
                    i++;
                }else if (input[i+1] == '"') {
                    i += 2;
                }else{
                    // 今いるところが「"」で、次の文字が「"」でない
                    break;
                }
            }
            if (input[i] != '"') {  // もしinputの最後もしくは「"」が「"」でないなら(「"」は「"」なので、inputの最後が「"」でない時のみ通る)
                tokens.push_back({ TokenType::Error, std::string(&input[start], i - start + 1)});
            }else{
                tokens.push_back({ TokenType::String, std::string(&input[start], i - start + 1) }); // 植木算
            }
            i++;
            continue;
        }

        tokens.push_back({ TokenType::Unknown, std::string(1, c) });
        i++;
    }
    return tokens;
}