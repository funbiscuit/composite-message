cmake_minimum_required(VERSION 3.8...3.17)

add_library(composite_message_lib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/composite_message.c
        )

target_include_directories(composite_message_lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_library(CompositeMessage::CompositeMessage ALIAS composite_message_lib)
