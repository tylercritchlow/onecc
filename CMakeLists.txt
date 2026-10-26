cmake_minimum_required(VERSION 3.20)
project(onecc VERSION 0.1.0 LANGUAGES C)

add_executable(onecc src/main.c)
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
endif()
