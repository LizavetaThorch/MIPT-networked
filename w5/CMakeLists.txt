cmake_minimum_required(VERSION 3.13)

project(w5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(W5_SOURCES
    main.cpp
    protocol.cpp
    entity.cpp
    )

set(W5_SERVER_SOURCES
    server.cpp
    protocol.cpp
    entity.cpp
    )


include_directories("../3rdParty/enet/include")

if(MSVC)
  # https://github.com/raysan5/raylib/issues/857
  add_compile_definitions(NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSERVICE NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
endif()

add_executable(w5 ${W5_SOURCES})
target_link_libraries(w5 PUBLIC project_options project_warnings)
target_link_libraries(w5 PUBLIC raylib enet)

add_executable(w5_server ${W5_SERVER_SOURCES})
target_link_libraries(w5_server PUBLIC project_options project_warnings)
target_link_libraries(w5_server PUBLIC enet)

if(MSVC)
  target_link_libraries(w5 PUBLIC ws2_32.lib winmm.lib)
  target_link_libraries(w5_server PUBLIC ws2_32.lib winmm.lib)
endif()

