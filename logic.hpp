#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <algorithm>
#include <fstream>
#include "TextEditor.h"



// ==========================================
// 1. エディタ言語定義
// =========================================

inline TextEditor::LanguageDefinition GetExcelDefinition() {
    TextEditor::LanguageDefinition Excellang;
    Excellang.mName = "Excel";
    Excellang.mCaseSensitive = false;
    Excellang.mSingleLineComment = "//";
    Excellang.mCommentStart = "/*";
    Excellang.mCommentEnd = "*/";

    // A. 制御構文
    const char* const controlKeywords[] = { "LAMBDA", "LET", "IF", "IFS", "SWITCH", "CHOOSE" };
    for (auto& k : controlKeywords) Excellang.mKeywords.insert(k);

    // B. excel_functions.csv から [関数名, 説明] を読み込む
    std::ifstream ifs("excel_functions.csv");
    std::string line;

    if (ifs.is_open()) {
        // ヘッダー行をスキップ
        std::getline(ifs, line);

        while (std::getline(ifs, line)) {
            if (line.empty()) continue;

            // 素朴なCSVパース (カンマで分割)
            // ※説明文の中にカンマが含まれる場合は、最初のカンマで分割する
            size_t comma_pos = line.find(',');
            if (comma_pos != std::string::npos) {
                std::string func_name = line.substr(0, comma_pos);
                std::string description = line.substr(comma_pos + 1);

                // CSVのクォート (") が残っている場合は除去
                if (!func_name.empty() && func_name.front() == '"') func_name = func_name.substr(1, func_name.size() - 2);
                if (!description.empty() && description.front() == '"') description = description.substr(1, description.size() - 2);

                TextEditor::Identifier id_data;
                // ここにCSVから取った説明文を入れる
                id_data.mDeclaration = description; 

                // 登録
                Excellang.mIdentifiers.insert({func_name, id_data});
            }
        }
    }

    // C. 演算子の登録
    const char* const operators[] = {
        "+", "-", "*", "/", "%", "^", 
        "=", ">", "<", ">=", "<=", "<>", 
        "&", ":", ",", "@", "#"
    };
    for (auto& o : operators){
        TextEditor::Identifier op_id;
        op_id.mDeclaration = "Operator";
        Excellang.mIdentifiers.insert(std::make_pair(std::string(o), op_id));
    }

    // トークン正規表現の追加
    // スペース（共通部分）を記号として扱うのは難しいので、まずは標準的な記号を優先
    Excellang.mTokenRegexStrings.push_back({ R"([\+\-\*\/\%\^\=\>\<\&\:\,\@\#]+)", TextEditor::PaletteIndex::Punctuation });


    // --- トークン定義 ---
    Excellang.mTokenRegexStrings.push_back({ R"("[^"\r\n]*")", TextEditor::PaletteIndex::String });
    Excellang.mTokenRegexStrings.push_back({ R"([+-]?[0-9]*\.?[0-9]+)", TextEditor::PaletteIndex::Number });
    Excellang.mTokenRegexStrings.push_back({ R"([a-zA-Z_][a-zA-Z0-9_\.]*)", TextEditor::PaletteIndex::Identifier });

    return Excellang;
}



// ==========================================
// 2. 字句解析 (Lexer)
// ==========================================

#include "lexer.hpp"

// ==========================================
// 3. 抽象構文木 (AST) のノード設計
// ==========================================

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::string format(int indent) const = 0;
};

// 単なるテキスト（変数、演算子、リテラル）
struct TextNode : public ASTNode {
    std::string text;
    TextNode(std::string t) : text(std::move(t)) {}
    std::string format(int) const override { return text; }
};

// 式（カンマで区切られる1つの引数。複数のテキストや関数を含む）
struct ExprNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> children;
    std::string format(int indent) const override {
        std::string res;
        for (const auto& child : children) res += child->format(indent);
        return res;
    }
};

// 関数呼び出し
struct FuncNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ExprNode>> args;

    FuncNode(std::string n) : name(std::move(n)) {}

    std::string format(int indent) const override {
        std::string res = name + "(";
        if (args.empty()) return res + ")";

        std::string ind_str(indent * 4, ' ');
        std::string next_ind_str((indent + 1) * 4, ' ');
        //std::string ind_str(indent * 1, '\t');
        //std::string next_ind_str((indent + 1) * 1, '\t');

        std::string upper_name = name;
        for (auto& c : upper_name) c = toupper(c);

        if (upper_name == "LAMBDA") {
            for (size_t i = 0; i < args.size(); ++i) {
                if (i == args.size() - 1) { // 最後の引数（計算本体）
                    res += "\n" + next_ind_str + args[i]->format(indent + 1) + "\n" + ind_str;
                } else { // 引数リスト
                    res += args[i]->format(indent + 1) + ", ";
                }
            }
        }
        else if (upper_name == "LET") {
            res += "\n";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i % 2 == 0) res += next_ind_str; // 変数名の行頭
                
                res += args[i]->format(indent + 1);
                
                if (i == args.size() - 1) {
                    res += "\n" + ind_str; // 最後の要素
                } else if (i % 2 == 1) {
                    res += ",\n"; // 値の終わり（改行）
                } else {
                    res += ", "; // 変数名の直後
                }
            }
        } 
        else { // その他の一般関数
            res += "\n";
            for (size_t i = 0; i < args.size(); ++i) {
                res += next_ind_str + args[i]->format(indent + 1);
                if (i < args.size() - 1) res += ",\n";
                else res += "\n" + ind_str;
            }
        }
        res += ")";
        return res;
    }
};



// ==========================================
// 4. 構文解析器 (Parser)
// ==========================================

class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

public:
    Parser(std::vector<Token> t) : tokens(std::move(t)) {}

    std::unique_ptr<ExprNode> parse() {
        if (pos < tokens.size() && tokens[pos].value == "=") pos++; // 先頭の=をスキップ
        return parse_expr();
    }

private:
    std::unique_ptr<ExprNode> parse_expr() {
        auto expr = std::make_unique<ExprNode>();

        while (pos < tokens.size() && tokens[pos].type != TokenType::Comma && tokens[pos].type != TokenType::RParen) {
            
            // 関数呼び出しの判定 (Identifier の次が '(' なら関数)
            if (tokens[pos].type == TokenType::Identifier && pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::LParen) {
                std::string func_name = tokens[pos].value;
                pos += 2; // 名前と '(' を消費
                
                auto func_node = std::make_unique<FuncNode>(func_name);

                // 引数のパース
                if (pos < tokens.size() && tokens[pos].type != TokenType::RParen) {
                    while (pos < tokens.size()) {
                        func_node->args.push_back(parse_expr());
                        
                        if (pos < tokens.size() && tokens[pos].type == TokenType::Comma) {
                            pos++; // カンマを消費して次の引数へ
                        } else if (pos < tokens.size() && tokens[pos].type == TokenType::RParen) {
                            break; // 閉じ括弧で終了
                        }
                    }
                }
                if (pos < tokens.size() && tokens[pos].type == TokenType::RParen) pos++; // ')' を消費
                expr->children.push_back(std::move(func_node));
            } 
            else {
                // それ以外は単なるテキストとして追加
                expr->children.push_back(std::make_unique<TextNode>(tokens[pos].value));
                pos++;
            }
        }
        return expr;
    }
};



// ==========================================
// 5. 外部インターフェース
// ==========================================

inline std::string minify_excel(const std::string& input) {
    std::string result;
    bool in_string = false;
    for (char c : input) {
        if (c == '"') in_string = !in_string;
        if (in_string) {
            result += c;
        } else if (!isspace(c)) {
            result += c;
        }
    }
    return result;
}

inline std::string format_excel(const std::string& input) {
    std::string minified = minify_excel(input);
    if (minified.empty()) return "";

    auto tokens = tokenize(minified);
    Parser parser(tokens);
    auto ast = parser.parse();
    
    // Excelの数式らしく先頭に = をつける
    return "=" + ast->format(0);
}