cmake_minimum_required(VERSION 3.22)
project(CSC453-partner C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCES
    Asgn2/lwp.c
    Asgn2/rr_scheduler.c
    Asgn2/queue.c
    Asgn2/magic64.S)

add_executable(test Asgn2/testing.c ${SOURCES})
add_executable(numbers Asgn2/numbersmain.c ${SOURCES})
