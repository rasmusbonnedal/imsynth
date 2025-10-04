add_executable(imsynth
	audio_engine.cpp
	audio_engine.h
    audio_graph.cpp
    audio_graph.h
    stb_hexwave.h
    imgui_window.h
	main.cpp
	main_window.cpp
    main_window.h
	midi_node.cpp
	midi_node.h
	node_window.cpp
	node_window.h
)

target_link_libraries(imsynth
	imgui_glfw
	imgui-node-editor
	miniaudio
	Winmm

)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT imsynth)
