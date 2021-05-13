add_library(tools SHARED tools.cpp)
message("${Boost_LIBRARIES}")
target_include_directories(tools SYSTEM PRIVATE
        ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_link_libraries(tools ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

