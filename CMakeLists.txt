cmake_minimum_required(VERSION 3.17.0)

project(RocketSim VERSION 2.1.0)

include(GNUInstallDirs)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)

file(GLOB_RECURSE ROCKETSIM_BULLET3_SOURCES libsrc/bullet3-3.24/*.cpp)

file(GLOB_RECURSE ROCKETSIM_SOURCES src/*.cpp)

add_library(RocketSim-static STATIC)
target_sources(RocketSim-static PRIVATE ${ROCKETSIM_BULLET3_SOURCES} ${ROCKETSIM_SOURCES})

if(NOT NINTENDO_SWITCH AND NOT NINTENDO_3DS)
	add_library(RocketSim SHARED)
	target_sources(RocketSim PRIVATE ${ROCKETSIM_BULLET3_SOURCES} ${ROCKETSIM_SOURCES})
endif()

foreach(ROCKETSIM_LIBRARY RocketSim RocketSim-static)
	if((${ROCKETSIM_LIBRARY} STREQUAL "RocketSim") AND (NINTENDO_SWITCH OR NINTENDO_3DS))
		continue()
	endif()

	if(IPO_SUPPORTED)
		set_target_properties(${ROCKETSIM_LIBRARY} PROPERTIES
		    INTERPROCEDURAL_OPTIMIZATION TRUE
			INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
			INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
			INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
			INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE
		)
	endif()

	target_compile_features(${ROCKETSIM_LIBRARY} PRIVATE cxx_std_20)

	if(MSVC)
		target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC /arch:AVX2)
		target_compile_definitions(${ROCKETSIM_LIBRARY} PUBLIC _USE_MATH_DEFINES)
	elseif(NOT NINTENDO_SWITCH AND NOT NINTENDO_3DS)
		target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC -msse4.1)
		#target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC --coverage)
		#target_link_options(${ROCKETSIM_LIBRARY} PUBLIC --coverage)
		target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC -fno-omit-frame-pointer)
		target_link_options(${ROCKETSIM_LIBRARY} PUBLIC -fno-omit-frame-pointer)
		target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC -Wno-stringop-overflow -Wno-volatile)
		target_link_options(${ROCKETSIM_LIBRARY} PUBLIC -Wno-stringop-overflow -Wno-volatile)
		#target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC -Wsuggest-override -Werror)
		target_compile_options(${ROCKETSIM_LIBRARY} PUBLIC -Wsuggest-final-types -Wsuggest-final-methods)
	endif()

	set_target_properties(${ROCKETSIM_LIBRARY} PROPERTIES C_VISIBILITY_PRESET hidden)
	set_target_properties(${ROCKETSIM_LIBRARY} PROPERTIES CXX_VISIBILITY_PRESET hidden)

	target_include_directories(${ROCKETSIM_LIBRARY} PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libsrc/bullet3-3.24>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	)
endforeach()

# only export explicitly
if(NOT NINTENDO_SWITCH AND NOT NINTENDO_3DS)
	target_compile_definitions(RocketSim PRIVATE ROCKETSIM_EXPORTS)
endif()
target_compile_definitions(RocketSim-static PUBLIC ROCKETSIM_STATIC)




if(NOT NINTENDO_SWITCH AND NOT NINTENDO_3DS)
	###########################
	### Example application ###
	###########################

	#find_package(RocketSim REQUIRED)
	find_package(Eigen3 REQUIRED)
	add_executable(RocketSimTest)
	target_compile_features(RocketSimTest PRIVATE cxx_std_20)

	target_sources(RocketSimTest PRIVATE test.cpp)

	# choose shared or static lib here
	#target_link_libraries(RocketSimTest PRIVATE RocketSim)
	target_link_libraries(RocketSimTest PRIVATE RocketSim-static Eigen3::Eigen)

	if(IPO_SUPPORTED)
		set_target_properties(RocketSimTest PROPERTIES
			INTERPROCEDURAL_OPTIMIZATION TRUE
			INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
			INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
			INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
			INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE
		)
	endif()

	set_target_properties(RocketSimTest PROPERTIES C_VISIBILITY_PRESET hidden)
	set_target_properties(RocketSimTest PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

if(NINTENDO_3DS)
add_subdirectory(visualize-3ds)
else()
add_subdirectory(visualize)
#add_subdirectory(visualize-vk)
#add_subdirectory(train_ppo)
#add_subdirectory(train_ppo2)
endif()
