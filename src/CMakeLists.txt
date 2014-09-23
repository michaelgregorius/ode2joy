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

set (ODE2JOY_SRCS
  main.cpp
  Joystick.cpp
  JoystickLinux.cpp
  MIDIInterfaceALSA.cpp
)

add_executable(ode2joy ${ODE2JOY_SRCS})

if (ODE2JOY_LINK_LIBRARIES)
  message (STATUS "Adding link libraries: ${ODE2JOY_LINK_LIBRARIES}")
  target_link_libraries (ode2joy ${ODE2JOY_LINK_LIBRARIES})
endif (ODE2JOY_LINK_LIBRARIES)

install (TARGETS ode2joy
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
