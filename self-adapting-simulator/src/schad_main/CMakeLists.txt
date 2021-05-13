add_executable(schad
        main.cpp
        schad/packet/packet_builder.cpp
        schad/configs/policy_config.cpp
        schad/configs/source_config.cpp
        schad/configs/experiment_config.cpp
        schad/configs/reward_config.cpp
        schad/infrastructure/infrastructure_impl.cpp
        schad/infrastructure/infrastructure.cpp
        schad/traffic/poisson_source.cpp
        schad/traffic/union_source.cpp
        schad/traffic/sequence_source.cpp
        schad/traffic/markov_source.cpp
        schad/traffic/null_source.cpp
        schad/policy/pq_policy.cpp
        schad/reward/weighted_throughput_reward.cpp
        schad/simulator/simulation.cpp
        schad/simulator/multi_run_stats_collector.cpp
    )

target_include_directories(schad SYSTEM PUBLIC
        ${Boost_INCLUDE_DIRS}
        ${nlohmann_json_INCLUDE_DIRS}
        )
target_include_directories(schad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(schad
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        schad_learning
        )
