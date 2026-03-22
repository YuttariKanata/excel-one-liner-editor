#include <iostream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "TextEditor.h"

#include "logic.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main() {
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Exceler", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // フォント設定 (お好みでサイズ調整)
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\consola.ttf", 18.0f * 1.5f);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // --- エディタ & バッファ準備 ---
    TextEditor editor;
    editor.SetLanguageDefinition(GetExcelDefinition()); // logic.hppの関数を呼ぶだけ
    
    // 言語定義をセットした後にパレットをカスタマイズ
    /*
    auto palette = TextEditor::GetDarkPalette();

    // 1. LAMBDA, LET などのキーワード (標準は青系)
    palette[(int)TextEditor::PaletteIndex::Keyword] = 0xffd19a67; 

    // 2. functions.txt にある既知の関数 (Identifier)
    // ここを「エメラルドグリーン」や「オレンジ」にすると見やすいです
    palette[(int)TextEditor::PaletteIndex::Identifier] = 0xfffadaaa; 
    palette[(int)TextEditor::PaletteIndex::KnownIdentifier] = 0xffafdcdc; 

    // 3. 文字列や数値もついでに調整（お好みで）
    palette[(int)TextEditor::PaletteIndex::String] = 0xff7c94c5; 
    palette[(int)TextEditor::PaletteIndex::Number] = 0xffabcdba;
    
    // 4. Operator (P)
    palette[(int)TextEditor::PaletteIndex::Punctuation] = 0xffd4d4d4;

    editor.SetPalette(palette);
    */


    auto palette = TextEditor::GetDarkPalette();

    // --- 画面構成要素の色 ---
    palette[(int)TextEditor::PaletteIndex::Background]              = 0xff1f1f1f; // 背景色 (濃いグレー)
    palette[(int)TextEditor::PaletteIndex::Cursor]                  = 0xffadafae; // カーソル
    palette[(int)TextEditor::PaletteIndex::Selection]               = 0xff784f26; // 選択範囲 (半透明の青)
    palette[(int)TextEditor::PaletteIndex::LineNumber]              = 0xff80766f; // 行番号
    palette[(int)TextEditor::PaletteIndex::CurrentLineFill]         = 0xff2d2d2d; // 現在行の強調
    palette[(int)TextEditor::PaletteIndex::CurrentLineEdge]         = 0xff454545; // 現在行の枠線

    // --- 文法要素の色 (VS Code風) ---
    palette[(int)TextEditor::PaletteIndex::Default]                 = 0xffd4d4d4; // 標準テキスト
    palette[(int)TextEditor::PaletteIndex::Keyword]                 = 0xffd19a67; // LAMBDA, LET等
    palette[(int)TextEditor::PaletteIndex::Number]                  = 0xffabcdba; // 数値 (薄緑)
    palette[(int)TextEditor::PaletteIndex::String]                  = 0xff7c94c5; // 文字列 (茶)
    palette[(int)TextEditor::PaletteIndex::Punctuation]             = 0xffd4d4d4; // 演算子・記号
    palette[(int)TextEditor::PaletteIndex::KnownIdentifier]         = 0xffafdcdc; // 既知の関数 (黄色系)
    palette[(int)TextEditor::PaletteIndex::Identifier]              = 0xfffadaaa; // 変数・セル参照 (水色)
    palette[(int)TextEditor::PaletteIndex::Comment]                 = 0xff5d9874; // コメント (緑)

    editor.SetPalette(palette);


    // 上部一行入力用のバッファ (char配列が必要)
    char inputBuffer[4096] = ""; 

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 画面全体を覆うウィンドウ
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("MainLayout", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        // 1. 上部：一行入力ボックス
        ImGui::Text("Input (One-liner):");
        ImGui::PushItemWidth(-1.0f); // 横幅いっぱいに広げる
        ImGui::InputText("##one_liner", inputBuffer, IM_ARRAYSIZE(inputBuffer));
        ImGui::PopItemWidth();

        ImGui::Spacing();

        // 2. 中段：ボタン2つ
        // ボタンを横に並べる
        // 左ボタン：上(Input) -> 下(Editor) へ整形して送る
        if (ImGui::Button("Format Down", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
            editor.SetText(format_excel(inputBuffer));
        }
        
        ImGui::SameLine();

        if (ImGui::Button("Minify Up", ImVec2(-1.0f, 0))) {
            std::string minified = minify_excel(editor.GetText());
            // char配列への書き戻しを安全に行う
            size_t copy_len = std::min(minified.size(), sizeof(inputBuffer) - 1);
            memcpy(inputBuffer, minified.c_str(), copy_len);
            inputBuffer[copy_len] = '\0';
        }

        ImGui::Separator();

        // 3. 下部：でかいテキストエディタ
        ImGui::Text("Editor (Formatted):");
        // 残りの画面領域をすべて使う
        editor.Render("TextEditor", ImGui::GetContentRegionAvail());

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}