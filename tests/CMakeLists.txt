enable_testing(true)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
set(TEST_COMMON_LIB
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
include_directories(../src/)

qt_add_executable(testdbcparser MANUAL_FINALIZATION
  testdbcparser.cpp
  ../src/dbcparser.cpp
  ../src/dbcparser.h)

target_link_libraries(testdbcparser PRIVATE ${TEST_COMMON_LIB})
add_test(NAME testdbcparser COMMAND testdbcparser)
qt_finalize_executable(testdbcparser)

qt_add_executable(testcanmsg MANUAL_FINALIZATION
  testcanmsg.cpp ../src/canmsg.h ../src/canmsg.cpp)
target_link_libraries(testcanmsg PRIVATE ${TEST_COMMON_LIB})
add_test(NAME testcanmsg COMMAND testcanmsg)
qt_finalize_executable(testcanmsg)

