cmake_minimum_required(VERSION 3.7)
project(trender VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

include_directories(
	"src/glm"
	${SDL2_INCLUDE_DIRS}
)

file(GLOB SRC
	"src/*.h"
	"src/*.cpp"
	"src/util/*.h"
	"src/util/*.cpp"
	"src/data/*.h"
	"src/data/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARIES}
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()