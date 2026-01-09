cmake_minimum_required (VERSION 3.16.3)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

enable_language(CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	set(CMAKE_CXX_FLAGS "/W4 /EHsc")
else()
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unknown-pragmas -pthread")
endif()

project(Prologue)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../application)

include_directories(
	SYSTEM "deps/include/"
	"src/"
)

set(SOURCE_FILES
	"src/app/AppSetting.cpp"
	"src/app/CommandLine.cpp"
	"src/app/FetchVersion.cpp"
	"src/app/Option.cpp"

	"src/dynamics/WindModel.cpp"

	"src/env/Environment.cpp"
	"src/env/Map.cpp"

	"src/gnuplot/plotter/IPlotter.cpp"
	"src/gnuplot/plotter/Plotter2D.cpp"
	"src/gnuplot/plotter/Plotter3D.cpp"

	"src/math/Algorithm.cpp"
	"src/math/Quaternion.cpp"
	"src/math/Vector3D.cpp"

	"src/result/ResultSaver.cpp"
	"src/result/SimuResult.cpp"

	"src/rocket/AeroCoefficient.cpp"
	"src/rocket/Engine.cpp"
	"src/rocket/RocketSpec.cpp"

	"src/simulator/DetailSimulator.cpp"
	"src/simulator/ScatterSimulator.cpp"
	"src/simulator/SimulatorBase.cpp"
	"src/simulator/SimulatorFactory.cpp"

	"src/solver/Solver.cpp"

	"src/utils/JsonUtils.cpp"

	"src/main.cpp"
)

add_executable(Prologue ${SOURCE_FILES})

add_subdirectory("test")