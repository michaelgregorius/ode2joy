cmake_minimum_required(VERSION 3.16)
project(ode2joy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Building on ${CMAKE_SYSTEM_NAME}...")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
pkg_check_modules(RTMIDI REQUIRED IMPORTED_TARGET rtmidi)

# Qt-free core: SDL3 input, mapping engine, RtMidi output
add_library(ode2joy_core STATIC
  MIDI/MIDIEvent.cpp
  MIDI/MIDIEvent.h
  core/ControlEvent.h
  core/InputBackend.h
  core/Mapping.h
  core/MappingEngine.cpp
  core/MappingEngine.h
  core/MidiOutput.h
  core/Pipeline.cpp
  core/Pipeline.h
  core/Processing.cpp
  core/Processing.h
  core/ProfileLibrary.cpp
  core/ProfileLibrary.h
  core/RtMidiOutput.cpp
  core/SdlInput.cpp
  core/SdlInput.h
)
target_include_directories(ode2joy_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/core)
target_link_libraries(ode2joy_core PUBLIC PkgConfig::SDL3 PkgConfig::RTMIDI Threads::Threads)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(ode2joy_core PRIVATE -Wall -Wno-long-long -pedantic)
endif()

# Define GUI classes
set (QT_GUI_HEADERS
  GUI/DeviceListWidget.h
  GUI/MainWindow.h
  GUI/PipelineBridge.h
  GUI/ProfileJson.h
  GUI/ProfileStore.h
)

set (QT_GUI_SRCS
  GUI/DeviceListWidget.cpp
  GUI/MainWindow.cpp
  GUI/PipelineBridge.cpp
  GUI/ProfileJson.cpp
  GUI/ProfileStore.cpp
)

set (QT_GUI_RCCS GUI/Resources/resources.qrc)

if (WIN32)
set (GUI_TYPE WIN32)
endif (WIN32)
# Set some Apple MacOS Specific settings
if (APPLE)
set (GUI_TYPE MACOSX_BUNDLE)
endif (APPLE)

add_executable(ode2joy
  ${GUI_TYPE}
  main.cpp
  ${QT_GUI_HEADERS}
  ${QT_GUI_SRCS}
  ${QT_GUI_RCCS}
)

set_target_properties(ode2joy PROPERTIES
  AUTOMOC ON
  AUTORCC ON
)
target_include_directories(ode2joy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ode2joy PRIVATE ode2joy_core Qt6::Widgets)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(ode2joy PRIVATE -Wall -Wno-long-long -pedantic)
endif()

install (TARGETS ode2joy
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Unit test section
enable_testing()

add_executable(MIDITest UnitTests/MIDITest.cpp)
target_link_libraries(MIDITest PRIVATE ode2joy_core)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(MIDITest PRIVATE -Wall -Wno-long-long -pedantic)
endif()

set(ODE2JOY_TESTS
  test_status
  test_channel
  test_note
  test_velocity
  test_polyphonic_aftertouch
  test_controller
  test_controller_value
  test_program_change
  test_channel_aftertouch
  test_pitch_bend
  test_dead_zone
  test_curves
  test_quantize
  test_axis_to_cc
  test_invert_and_dead_zone
  test_unipolar_axis
  test_change_suppression
  test_rate_limit_and_flush
  test_smoothing
  test_pitch_bend_target
  test_button_note_tracking
  test_release_on_remove
  test_hat_mappings
  test_pipeline_disconnect_releases_notes
  test_profile_library_resolution
)

foreach(t ${ODE2JOY_TESTS})
  add_test(NAME ${t} COMMAND MIDITest ${t})
endforeach()
