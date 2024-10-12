cmake_minimum_required(VERSION 3.13)

project(w4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(W4_SOURCES
    main.cpp
    protocol.cpp
    )

set(W4_SERVER_SOURCES
    server.cpp
    protocol.cpp
    )


include_directories("../3rdParty/enet/include")

if(MSVC)
  # https://github.com/raysan5/raylib/issues/857
  add_compile_definitions(NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSERVICE NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
endif()

add_executable(w4 ${W4_SOURCES})
target_link_libraries(w4 PUBLIC project_options project_warnings)
target_link_libraries(w4 PUBLIC raylib enet)

add_executable(w4_server ${W4_SERVER_SOURCES})
target_link_libraries(w4_server PUBLIC project_options project_warnings)
target_link_libraries(w4_server PUBLIC enet)

if(MSVC)
  target_link_libraries(w4 PUBLIC ws2_32.lib winmm.lib)
  target_link_libraries(w4_server PUBLIC ws2_32.lib winmm.lib)
endif()
