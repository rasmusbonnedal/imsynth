add_executable(imsynth
	audio_engine.cpp
	audio_engine.h
    imgui_window.h
	main.cpp
	main_window.cpp
    main_window.h
)

target_link_libraries(imsynth
	imgui_glfw
	imgui-node-editor
	miniaudio
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT imsynth)
