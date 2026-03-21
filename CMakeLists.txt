cmake_minimum_required(VERSION 3.15)
project(ExcelerImGui CXX)

set(CMAKE_CXX_STANDARD 17)

# GLFWのビルド設定（ドキュメント生成などは不要なのでOFFに）
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)

# ImGuiのソースファイル指定
set(IMGUI_DIR extern/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImGuiColorTextEdit
set(TEXT_EDIT_DIR extern/ImGuiColorTextEdit)

add_executable(${PROJECT_NAME} 
    main.cpp 
    ${IMGUI_SOURCES}
    ${TEXT_EDIT_DIR}/TextEditor.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    ${TEXT_EDIT_DIR}
    extern/glfw/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw opengl32)