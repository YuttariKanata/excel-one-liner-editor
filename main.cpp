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