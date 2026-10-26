cmake_minimum_required(VERSION 3.16)
project(ode2joy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Building on ${CMAKE_SYSTEM_NAME}...")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)

set(ODE2JOY_HAVE_ALSA OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(ALSA QUIET)
  if(ALSA_FOUND)
    set(ODE2JOY_HAVE_ALSA ON)
    message(STATUS "ALSA found; enabling the ALSA MIDI driver")
  else()
    message(STATUS "ALSA development files not found; building without the ALSA MIDI driver")
  endif()
endif()

set (ODE2JOY_SRCS
  Drivers/Joystick/JoystickDriverFactory.cpp
  Drivers/Joystick/JoystickDriverLinux.cpp
  Drivers/Joystick/JoystickDriverNull.cpp
  Drivers/MIDI/MIDIDriverFactory.cpp
  Drivers/MIDI/MIDIDriverNull.cpp
  MIDI/MIDIEvent.cpp
  JoystickDriver.cpp
  Joystick.cpp
  JoystickEvent.cpp
  JoystickToMIDIMapper.cpp
  Session.cpp
  SessionObserver.cpp
)

set (ODE2JOY_HEADERS
  Drivers/Joystick/JoystickDriverFactory.h
  Drivers/Joystick/JoystickDriverLinux.h
  Drivers/Joystick/JoystickDriverNull.h
  Drivers/MIDI/MIDIDriverFactory.h
  Drivers/MIDI/MIDIDriverNull.h
  Drivers/DriverDescription.h
  MIDI/MIDIEvent.h
  JoystickDriver.h
  MIDIDriver.h
  JoystickToMIDIMapper.h
  Session.h
  SessionObserver.h
)

if(ODE2JOY_HAVE_ALSA)
  list(APPEND ODE2JOY_SRCS Drivers/MIDI/MIDIDriverALSA.cpp)
  list(APPEND ODE2JOY_HEADERS Drivers/MIDI/MIDIDriverALSA.h)
endif()

# Define GUI classes
set (QT_GUI_HEADERS
  GUI/MainWindow.h
  GUI/SessionWidget.h
)

set (QT_GUI_SRCS
  GUI/MainWindow.cpp
  GUI/SessionWidget.cpp
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
  ${ODE2JOY_SRCS}
  ${ODE2JOY_HEADERS}
  ${QT_GUI_HEADERS}
  ${QT_GUI_SRCS}
  ${QT_GUI_RCCS}
)

set_target_properties(ode2joy PROPERTIES
  AUTOMOC ON
  AUTORCC ON
)
target_include_directories(ode2joy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ode2joy PRIVATE Qt6::Widgets Threads::Threads)

if(ODE2JOY_HAVE_ALSA)
  target_compile_definitions(ode2joy PRIVATE ODE2JOY_HAVE_ALSA)
  target_link_libraries(ode2joy PRIVATE ALSA::ALSA)
endif()

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

add_executable(MIDITest
  UnitTests/MIDITest.cpp
  MIDI/MIDIEvent.cpp
  Drivers/MIDI/MIDIDriverFactory.cpp
  Drivers/MIDI/MIDIDriverNull.cpp
)
target_include_directories(MIDITest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(ODE2JOY_HAVE_ALSA)
  target_sources(MIDITest PRIVATE Drivers/MIDI/MIDIDriverALSA.cpp)
  target_compile_definitions(MIDITest PRIVATE ODE2JOY_HAVE_ALSA)
  target_link_libraries(MIDITest PRIVATE ALSA::ALSA)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(MIDITest PRIVATE -Wall -Wno-long-long -pedantic)
endif()

add_test(MIDI-Status MIDITest test_status)
add_test(MIDI-Channel MIDITest test_channel)
add_test(MIDI-Note MIDITest test_note)
add_test(MIDI-Velocity MIDITest test_velocity)
add_test(MIDI-PolyphonicAftertouch MIDITest test_polyphonic_aftertouch)
add_test(MIDI-Controller MIDITest test_controller)
add_test(MIDI-ControllerValue MIDITest test_controller_value)
add_test(MIDI-ProgramChange MIDITest test_program_change)
add_test(MIDI-ChannelAftertouch MIDITest test_channel_aftertouch)
add_test(MIDI-Pitchbend MIDITest test_pitch_bend)
add_test(MIDI-DriverFactory MIDITest test_driver_factory)
