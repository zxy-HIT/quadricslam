project(quadricslam)
cmake_minimum_required(VERSION 3.0)

set(QUADRICSLAM_MAJOR_VERSION 0)
set(QUADRICSLAM_MINOR_VERSION 1)
set(QUADRICSLAM_PATCH_VERSION 0)
set(QUADRICSLAM_VERSION
  ${QUADRICSLAM_MAJOR_VERSION}.${QUADRICSLAM_MINOR_VERSION}.${QUADRICSLAM_PATCH_VERSION})


# we want to use some C++11 features
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

###################################################################################
# set up options
option(QSLAM_BUILD_PYTHON_WRAP "Enable/Disable building of cython wrapper" OFF)
set(QSLAM_PYTHON_VERSION "3" CACHE STRING "The version of python to build the cython wrapper for (or Default)")



###################################################################################
# find GTSAM and GTSAM CMake tools
find_package(GTSAM REQUIRED)
find_package(GTSAMCMakeTools REQUIRED)

# set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${GTSAM_DIR}/../GTSAMCMakeTools")

# include gtsam cmake tools
include(GtsamBuildTypes) 			# Load build type flags and default to Debug mode
include(GtsamTesting)    			# Easy functions for creating unit tests and scripts
include(GtsamMakeConfigFile) 	# Defines a function for generating config file so package can be found with find_package function

# NOTE: PYTHONPATH must include GTSAM_CYTHON_INSTALL_PATH for gtsam/gtsam.pxd
if(QSLAM_BUILD_PYTHON_WRAP)
	# python version must be set before including gtsamcythonwrap
	set(GTSAM_PYTHON_VERSION ${QSLAM_PYTHON_VERSION})
	include(GtsamMatlabWrap) # Automatic MATLAB wrapper generation
	include(GtsamCythonWrap) # Automatic MATLAB wrapper generation
endif()

###################################################################################
# add quadricslam

# include headers, ensure that local folder is searched before library folders
include_directories(BEFORE "${PROJECT_SOURCE_DIR}")

# set source files
set(SOURCE_FILES
	./quadricslam/base/Utilities.cpp
	./quadricslam/geometry/ConstrainedDualQuadric.cpp
	./quadricslam/geometry/AlignedBox2.cpp
	./quadricslam/geometry/AlignedBox3.cpp
	./quadricslam/geometry/BoundingBoxFactor.cpp
	./quadricslam/geometry/QuadricCamera.cpp
	./quadricslam/geometry/DualConic.cpp
)

###################################################################################
## Build static library from common sources

set(CONVENIENCE_LIB_NAME "quadricslam")
add_library(${CONVENIENCE_LIB_NAME} SHARED ${SOURCE_FILES})
# set_target_properties(${CONVENIENCE_LIB_NAME} PROPERTIES PREFIX "")
target_link_libraries(${CONVENIENCE_LIB_NAME} gtsam)

###################################################################################
# install library and PACKAGEConfig.cmake
install(
	TARGETS ${CONVENIENCE_LIB_NAME} 
	EXPORT QUADRICSLAM-exports
	LIBRARY DESTINATION lib 
	ARCHIVE DESTINATION lib 
	RUNTIME DESTINATION bin)
GtsamMakeConfigFile(QUADRICSLAM)
# list(APPEND GTSAM_EXPORTED_TARGETS ${CONVENIENCE_LIB_NAME})
# export(TARGETS ${GTSAM_EXPORTED_TARGETS} FILE GTSAM-exports.cmake)

file(GLOB quadricslam_base_headers "quadricslam/base/*.h")
install(FILES ${quadricslam_base_headers} DESTINATION include/quadricslam/base)
file(GLOB quadricslam_geometry_headers "quadricslam/geometry/*.h")
install(FILES ${quadricslam_geometry_headers} DESTINATION include/quadricslam/geometry)

###################################################################################
# uninstall target 
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

###################################################################################
# Build tests (CMake tracks the dependecy to link with GTSAM through our project's static library)
gtsamAddTestsGlob("geometry" "quadricslam/geometry/tests/test*.cpp" "" "${CONVENIENCE_LIB_NAME}")
gtsamAddTestsGlob("geometry" "quadricslam/base/tests/test*.cpp" "" "${CONVENIENCE_LIB_NAME}")

###################################################################################
# Build example files (CMake tracks the dependecy to link with GTSAM through our project's static library)
# gtsamAddExamplesGlob("*.cpp" "" "${CONVENIENCE_LIB_NAME}")
# build examples if make examples
# https://stackoverflow.com/questions/17765618/cmake-add-subdirectories-to-custom-target-and-adding-custom-target-to-vs-solut
# TODO: pass ${CONVENIENCE_LIB_NAME} to examples/c++/CMakeLists.txt
# add_custom_target(examples) <-- already exists from GtsamTesting
add_subdirectory(examples/c++)
add_dependencies(examples examples_folder)

###################################################################################
# Build MATLAB wrapper (CMake tracks the dependecy to link with GTSAM through our project's static library)
# wrap_and_install_library("example.h" "${CONVENIENCE_LIB_NAME}" "" "")

###################################################################################
# Build Cython wrapper (CMake tracks the dependecy to link with GTSAM through our project's static library)
if(QSLAM_BUILD_PYTHON_WRAP)

	# include gtsam eigency (handles numpy->eigen conversion)
	include_directories(${GTSAM_EIGENCY_INSTALL_PATH})

	# create __init__.py to allow build/cython/quadricslam be imported as a python package
	file(WRITE "${PROJECT_BINARY_DIR}/cython/quadricslam/__init__.py" "from .quadricslam import *")

	set(QUADRICSLAM_CYTHON_INSTALL_PATH "" CACHE PATH "Cython toolbox destination, blank defaults to CMAKE_INSTALL_PREFIX/cython")
	if (NOT QUADRICSLAM_CYTHON_INSTALL_PATH)
		set(QUADRICSLAM_CYTHON_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/cython/quadricslam")
	endif()

	# use gtsams cython wrapping functionality
	wrap_and_install_library_cython("./quadricslam.h" # interface_header path
									"from gtsam.gtsam cimport *" # extra imports for cython pxd
									"${QUADRICSLAM_CYTHON_INSTALL_PATH}" # install_path
									"gtsam;${CONVENIENCE_LIB_NAME}" # libraries to link with
									"wrap;gtsam;${CONVENIENCE_LIB_NAME}" # dependencies)
	)

	# install __init__.py so package can be loaded
	# install_cython_files("${PROJECT_BINARY_DIR}/cython/quadricslam/__init__.py" "${QUADRICSLAM_CYTHON_INSTALL_PATH}")
endif()

###################################################################################
# Build Doxygen documentation 

# find doxygen
find_package(Doxygen)

# add subdirectory with custom make doc, doc_clean targets
if (DOXYGEN_FOUND)
	add_subdirectory(doc)
endif()
