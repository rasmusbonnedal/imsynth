add_executable(imsynth
	imguiwindow.h
	main.cpp
	main_window.cpp
    main_window.h
)

target_link_libraries(imsynth
	imgui_glfw
	imgui-node-editor
)
