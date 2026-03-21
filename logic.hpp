#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include "TextEditor.h"

// --- 言語定義の集約 ---
inline TextEditor::LanguageDefinition GetExcelDefinition() {
    TextEditor::LanguageDefinition Excellang;
    Excellang.mName = "Excel";
    Excellang.mCaseSensitive = false;
    
    // ダミーコメントでステートマシンを安定させる
    Excellang.mSingleLineComment = "//";
    Excellang.mCommentStart = "/*";
    Excellang.mCommentEnd = "*/";

    const char* const keywords[] = { "LAMBDA", "LET", "IF", "IFS", "FILTER", "MAP", "REDUCE", "SCAN" };
    for (auto& k : keywords) Excellang.mKeywords.insert(k);

    // 文字列、数値、識別子のルール
    Excellang.mTokenRegexStrings.push_back({ R"("[^"\r\n]*")", TextEditor::PaletteIndex::String });
    Excellang.mTokenRegexStrings.push_back({ R"([+-]?[0-9]*\.?[0-9]+)", TextEditor::PaletteIndex::Number });
    Excellang.mTokenRegexStrings.push_back({ R"([a-zA-Z_][a-zA-Z0-9_\.]*)", TextEditor::PaletteIndex::Identifier });
    
    return Excellang;
}



// --- Minify: 空白・改行を一切許さない ---
inline std::string minify_excel(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    bool in_string = false;

    for (char c : input) {
        if (c == '"') in_string = !in_string;

        if (in_string) {
            result += c; // 文字列の中は死守
        } else {
            // 文字列外なら、空白・改行・タブをすべて無視
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                result += c;
            }
        }
    }
    return result;
}

// --- Format: switch文による構造化 ---
inline std::string format_excel(const std::string& input) {
    std::string minified = minify_excel(input);
    std::string result;
    result.reserve(minified.size() * 2);
    
    int indent_level = 0;
    bool in_string = false;

    auto append_indent = [&](int level) {
        for (int i = 0; i < level; ++i) result += "    ";
    };

    for (size_t i = 0; i < minified.length(); ++i) {
        char c = minified[i];

        // 文字列の中なら素通り（switchの前に判定）
        if (in_string) {
            result += c;
            if (c == '"') in_string = false;
            continue;
        }

        switch (c) {
            case '"':
                in_string = true;
                result += c;
                break;

            case '(':
                result += "(\n";
                indent_level++;
                append_indent(indent_level);
                break;

            case ')':
                result += "\n";
                indent_level = (indent_level > 0) ? indent_level - 1 : 0;
                append_indent(indent_level);
                result += ")";
                break;

            case ',':
                result += ",\n";
                append_indent(indent_level);
                break;

            default:
                result += c;
                break;
        }
    }
    return result;
}