cmake_minimum_required(VERSION 3.20)
project(onecc VERSION 0.1.0 LANGUAGES C)

add_library(assembler STATIC
    src/assembler/lexer.c
    src/assembler/parser.c
    src/assembler/symbols.c
    src/assembler/assembler.c
)
set_target_properties(assembler PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

add_executable(onecc src/main.c src/file.c src/vm/vm_subleq.c)
target_link_libraries(onecc PRIVATE assembler)
set_target_properties(onecc PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

install(TARGETS onecc RUNTIME DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
    find_path(GREATEST_INCLUDE_DIR NAMES greatest.h REQUIRED)
    add_executable(test_vm tests/test_vm.c src/vm/vm_subleq.c)
    target_include_directories(test_vm PRIVATE src "${GREATEST_INCLUDE_DIR}")
    set_target_properties(test_vm PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    add_test(NAME vm COMMAND test_vm)
    set_tests_properties(vm PROPERTIES TIMEOUT 5)
    set(ONECC_TEST_INSTALL_DIR "libexec/onecc-tests" CACHE PATH
        "Installation directory for test executables")
    install(TARGETS test_vm RUNTIME DESTINATION "${ONECC_TEST_INSTALL_DIR}")
    add_executable(test_lexer tests/test_lexer.c src/assembler/lexer.c)
    target_include_directories(test_lexer PRIVATE src "${GREATEST_INCLUDE_DIR}")
    set_target_properties(test_lexer PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    add_test(NAME lexer COMMAND test_lexer)
    set_tests_properties(lexer PROPERTIES TIMEOUT 5)
    install(TARGETS test_lexer RUNTIME DESTINATION "${ONECC_TEST_INSTALL_DIR}")
    add_executable(test_parser tests/test_parser.c src/assembler/parser.c src/assembler/lexer.c)
    target_include_directories(test_parser PRIVATE src "${GREATEST_INCLUDE_DIR}")
    set_target_properties(test_parser PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    add_test(NAME parser COMMAND test_parser)
    set_tests_properties(parser PROPERTIES TIMEOUT 5)
    install(TARGETS test_parser RUNTIME DESTINATION "${ONECC_TEST_INSTALL_DIR}")
    add_executable(test_assembler tests/test_assembler.c src/vm/vm_subleq.c)
    target_link_libraries(test_assembler PRIVATE assembler)
    target_include_directories(test_assembler PRIVATE src "${GREATEST_INCLUDE_DIR}")
    set_target_properties(test_assembler PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    add_test(NAME assembler COMMAND test_assembler)
    set_tests_properties(assembler PROPERTIES TIMEOUT 5)
    install(TARGETS test_assembler RUNTIME DESTINATION "${ONECC_TEST_INSTALL_DIR}")
endif()
