cmake_minimum_required (VERSION 2.6)
project (ode2joy)

message(STATUS "Building on ${CMAKE_SYSTEM_NAME}...")

## Find packages
find_package (Qt4 COMPONENTS QtCore QtGui REQUIRED)
include (${QT_USE_FILE})

set (ODE2JOY_LINK_LIBRARIES ${ODE2JOY_LINK_LIBRARIES} ${QT_LIBRARIES})

set (LINK_ALSA ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (${LINK_ALSA})
  message (STATUS "Checking for ALSA dev files...")
  find_package (ALSA REQUIRED)
  if (ALSA_FOUND)
    set (ODE2JOY_LINK_LIBRARIES ${ODE2JOY_LINK_LIBRARIES} ${ALSA_LIBRARY})
  endif (ALSA_FOUND)
endif (${LINK_ALSA})


## Define include directories
include_directories (${CMAKE_CURRENT_SOURCE_DIR}) # Used for includes from the base directory

# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
include_directories (${CMAKE_CURRENT_BINARY_DIR})


## Enable all warnings for GCC
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-long-long -pedantic")
endif()

set (ODE2JOY_SRCS
  Drivers/Joystick/JoystickDriverFactory.cpp
  Drivers/Joystick/JoystickDriverLinux.cpp
  Drivers/Joystick/JoystickDriverNull.cpp
  Drivers/MIDI/MIDIDriverALSA.cpp
  Drivers/MIDI/MIDIDriverFactory.cpp
  Drivers/MIDI/MIDIDriverNull.cpp
  main.cpp
  JoystickDriver.cpp
  Joystick.cpp
  JoystickToMIDIMapper.cpp
  Session.cpp
)

set (ODE2JOY_HEADERS
  Drivers/Joystick/JoystickDriverFactory.h
  Drivers/Joystick/JoystickDriverLinux.h
  Drivers/Joystick/JoystickDriverNull.h
  Drivers/MIDI/MIDIDriverALSA.h
  Drivers/MIDI/MIDIDriverFactory.h
  Drivers/MIDI/MIDIDriverNull.h
  Drivers/DriverDescription.h
  JoystickDriver.h
  MIDIDriver.h
  JoystickToMIDIMapper.h
  Session.h
)

# Define GUI classes
set (QT_GUI_MOC_HEADERS
  GUI/MainWindow.h
)

set (QT_GUI_SRCS
  GUI/MainWindow.cpp
)

SET (QT_GUI_UIS
)

# generate rules for building source files from the resources
set (QT_GUI_RCCS GUI/Resources/resources.qrc)
qt4_add_resources (QT_GUI_RCC_SRCS ${QT_GUI_RCCS})

# MOC
qt4_wrap_cpp (QT_GUI_MOC_OUTFILES ${QT_GUI_MOC_HEADERS})

# UI files
qt4_wrap_ui (QT_GUI_UIS_H ${QT_GUI_UIS})

if (WIN32)
set (GUI_TYPE WIN32)
endif (WIN32)
# Set some Apple MacOS Specific settings
if (APPLE)
set (GUI_TYPE MACOSX_BUNDLE)
endif (APPLE)

add_executable(ode2joy
               ${GUI_TYPE}
               ${ODE2JOY_SRCS}
               ${ODE2JOY_HEADERS}
               ${QT_GUI_SRCS}
               ${QT_GUI_MOC_OUTFILES}
               ${QT_GUI_RCC_SRCS}
               ${QT_GUI_UIS_H})

if (ODE2JOY_LINK_LIBRARIES)
  message (STATUS "Adding link libraries: ${ODE2JOY_LINK_LIBRARIES}")
  target_link_libraries (ode2joy ${ODE2JOY_LINK_LIBRARIES})
endif (ODE2JOY_LINK_LIBRARIES)

install (TARGETS ode2joy
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
