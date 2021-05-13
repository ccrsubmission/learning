add_library(schad_learning SHARED
        schad/learning/constant.cpp
        schad/learning/ucb.cpp
        schad/learning/ucb_e.cpp
        schad/learning/combined.cpp
        schad/learning/restarting.cpp
        schad/learning/dgp_ucb.cpp
        schad/learning/softmax.cpp
        schad/learning/normalized.cpp
        schad/learning/scaled.cpp
        schad/learning/ucb_tuned.cpp
        schad/learning/epsilon_greedy.cpp
        schad/learning/learning_config.cpp
        schad/learning/ucb_v.cpp
        schad/learning/uniform_identification.cpp
        schad/learning/explore_exploit.cpp
        schad/learning/successive_rejects.cpp
        schad/learning/local_greedy.cpp
        )

target_include_directories(schad_learning SYSTEM PUBLIC
        ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(schad_learning schad_common)
