cmake_minimum_required(VERSION 3.16)
project(newsdp C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(newsdp
        helpdesk.c
        helpdesk_state.c
        bst.c
        heap.c
        stack.c
        ticket_service.c
        engineer_service.c
        json_output.c
        commands.c
        helpdesk.h)
