add_executable(composite_message_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/CompositeMessageTest.cpp
        )

target_include_directories(composite_message_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(composite_message_tests PRIVATE
        Catch2::Catch2WithMain CompositeMessage::CompositeMessage)
