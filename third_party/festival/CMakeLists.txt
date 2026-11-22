add_executable(festival
    advanced_deadlock.cpp
    advisors.cpp
    bfs.cpp
    biconnect.cpp
    board.cpp
    corral.cpp
    corral_deadlock.cpp
    cycle_deadlock.cpp
    deadlock.cpp
    deadlock_cache.cpp
    deadlock_utils.cpp
    debug.cpp
    diags.cpp
    distance.cpp
    dragonfly.cpp
    dragonfly_queue.cpp
    engine.cpp
    envelope.cpp
    fixed_boxes.cpp
    girl.cpp
    global.cpp
    graph.cpp
    greedy.cpp
    helper.cpp
    heuristics.cpp
    hf_search.cpp
    holes.cpp
    hotspot.cpp
    hungarian.cpp
    imagine.cpp
    io.cpp
    k_dist_deadlock.cpp
    lurd.cpp
    match_distance.cpp
    max_dist.cpp
    mini_search.cpp
    moves.cpp
    mpdb2.cpp
    naive.cpp
    oop.cpp
    opener.cpp
    order.cpp
    overlap.cpp
    packing_search.cpp
    park_order.cpp
    perimeter.cpp
    positions.cpp
    preprocess.cpp
    queue.cpp
    request.cpp
    rooms.cpp
    rooms_deadlock.cpp
    score.cpp
    snail.cpp
    sokoban_solver.cpp
    sol.cpp
    stuck.cpp
    textfile.cpp
    tree.cpp
    util.cpp
    wobblers.cpp
    xy_deadlock.cpp
)

target_compile_definitions(festival PRIVATE FESTIVAL_NONINTERACTIVE)

if(WIN32)
    if(MSVC)
        target_compile_definitions(festival PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(festival PRIVATE /O2 /w)
    else()
        find_package(Threads REQUIRED)
        target_compile_definitions(festival PRIVATE THREADS)
        target_compile_options(festival PRIVATE -O2 -w)
        target_link_libraries(festival PRIVATE Threads::Threads)
    endif()
else()
    find_package(Threads REQUIRED)
    target_compile_definitions(festival PRIVATE LINUX THREADS)
    target_compile_options(festival PRIVATE -O2 -w)
    target_link_libraries(festival PRIVATE Threads::Threads)
endif()
