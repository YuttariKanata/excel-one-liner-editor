#include <iostream>
#include <string>
#include <vector>

// ImGui & Backends
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLFW
#include <GLFW/glfw3.h>

// TextEditor (extern/ImGuiColorTextEdit)
#include "TextEditor.h"

// format_excel_one_liner
#include "logic.hpp"


// エラーコールバック
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}



int main() {
    // 1. GLFW の初期化
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // OpenGL 3.3 Core Profile
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ウィンドウ作成
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Exceler - ImGui Edition", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync 有効

    // 2. ImGui コンテキストの初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    // フォントサイズを2.0倍にする（環境に合わせて調整してください）
    float baseFontSize = 18.0f; 
    // WindowsのConsolasを使う例（パスは環境に合わせて）
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\consola.ttf", baseFontSize * 1.5f);

    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // キーボード操作有効

    // フォント設定（日本語を表示したい場合は後で調整しましょう）
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\consola.ttf", 18.0f);

    // スタイル設定（Darkモード）
    ImGui::StyleColorsDark();

    // プラットフォーム/レンダラーのバックエンド初期化
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 3. TextEditor のセットアップ
    TextEditor editor;
    // --- Excel 言語定義の作成 ---
    // --- Excel 言語定義の修正案 ---
    // --- 徹底的にクリーンな Excel 定義 ---
    // --- 修正版：Excel 言語定義 ---
    // --- 決定版：Excel 言語定義 (状態リセット対策) ---
    TextEditor::LanguageDefinition langExcel;
    langExcel.mName = "Excel";
    langExcel.mCaseSensitive = false;

    // 1. コメント定義（ここが空だと、内部で「どこまでがコメントか」の判定がバグることがあります）
    // Excelには本来ありませんが、使わない記号を入れておきます
    langExcel.mSingleLineComment = "//"; // もし // を使いたくなければ "###" など
    langExcel.mCommentStart = "/*";
    langExcel.mCommentEnd = "*/";

    // 2. キーワード
    const char* const excelKeywords[] = {
        "LAMBDA", "LET", "IF", "IFS", "VLOOKUP", "XLOOKUP", "FILTER", "MAP", "REDUCE", "SCAN"
    };
    for (auto& k : excelKeywords) langExcel.mKeywords.insert(k);

    // 3. トークンルール
    langExcel.mTokenRegexStrings.clear();
    // 文字列 (最優先)
    langExcel.mTokenRegexStrings.push_back({ R"("[^"\r\n]*")", TextEditor::PaletteIndex::String });
    // 数値
    langExcel.mTokenRegexStrings.push_back({ R"([+-]?[0-9]*\.?[0-9]+([eE][+-]?[0-9]+)?)", TextEditor::PaletteIndex::Number });
    // 識別子
    langExcel.mTokenRegexStrings.push_back({ R"([a-zA-Z_][a-zA-Z0-9_\.]*)", TextEditor::PaletteIndex::Identifier });
    // 括弧やカンマ (これらにも色を付けたい場合)
    langExcel.mTokenRegexStrings.push_back({ R"([()\[\],;])", TextEditor::PaletteIndex::Punctuation });

    editor.SetLanguageDefinition(langExcel);


    editor.SetPalette(TextEditor::GetDarkPalette());

    // メインループ
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ImGui フレーム開始
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- GUI Logic ---
        
        // 画面いっぱいにウィンドウを配置
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        //ImGui::Text("Excel One-liner Formatter Prototype (ImGui)a");
        if (ImGui::Button("Format (Excel One-liner)")) {
            std::string input = editor.GetText();
            // ここであなたの logic.hpp の format 関数を呼ぶ！
            std::string output = format_excel_one_liner(input);
            editor.SetText(output);
        }
        ImGui::Separator();

        // エディタの描画
        editor.Render("TextEditor");

        ImGui::End();
        // ------------------

        // レンダリング
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 後片付け
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}