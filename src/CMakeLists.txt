cmake_minimum_required (VERSION 2.6)
project (ode2joy)

message(STATUS "Building on ${CMAKE_SYSTEM_NAME}...")

set (LINK_ALSA ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (${LINK_ALSA})
  message (STATUS "Checking for ALSA dev files...")
  find_package (ALSA REQUIRED)
  if (ALSA_FOUND)
    set (ODE2JOY_LINK_LIBRARIES ${ODE2JOY_LINK_LIBRARIES} ${ALSA_LIBRARY})
  endif (ALSA_FOUND)
endif (${LINK_ALSA})

# Define include directories
include_directories (${CMAKE_CURRENT_SOURCE_DIR}) # Used for includes from the base directory

# Enable all warnings for GCC
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-long-long -pedantic")
endif()

set (ODE2JOY_SRCS
  main.cpp
  JoystickDriver.cpp
  Drivers/Joystick/JoystickDriverLinux.cpp
  Joystick.cpp
  JoystickToMIDIMapper.cpp
  MIDIInterfaceALSA.cpp
)

set (ODE2JOY_HEADERS
  JoystickDriver.h
  MIDIInterface.h
  JoystickToMIDIMapper.h
  Drivers/Joystick/JoystickDriverLinux.h
)

add_executable(ode2joy ${ODE2JOY_SRCS} ${ODE2JOY_HEADERS})

if (ODE2JOY_LINK_LIBRARIES)
  message (STATUS "Adding link libraries: ${ODE2JOY_LINK_LIBRARIES}")
  target_link_libraries (ode2joy ${ODE2JOY_LINK_LIBRARIES})
endif (ODE2JOY_LINK_LIBRARIES)

install (TARGETS ode2joy
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
