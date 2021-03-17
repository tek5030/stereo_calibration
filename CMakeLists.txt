cmake_minimum_required(VERSION 3.5.1)
project(stereo_calibration_app)

# Compiler flags
if (MSVC)
  set(compiler_options /W4)
  add_definitions(/D_USE_MATH_DEFINES)
else ()
  set(compiler_options -Wall -Wextra -Wpedantic)
endif (MSVC)

# Add libraries.
find_package(OpenCV 4 REQUIRED)
find_package(tek5030 CONFIG REQUIRED)

# Add executable for stereo calibration app.
add_executable(${PROJECT_NAME}
  stereo_calib.h
  calibration_app.cpp
  stereo_calib.cpp
  )

target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  tek5030::tek5030
  )

target_compile_options(${PROJECT_NAME} PRIVATE ${compiler_options})
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 14
  )
