INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../boolector/src)

add_library(KINT MODULE
    # List your source files here.
    CheckInsertion.cpp
    CompilerAttribute.h
    Constraints.cpp
    Constraints.h
    SMTQuery.cpp
    SMTSolver.h
)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(KINT PUBLIC cxx_std_14)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(KINT PROPERTIES
    COMPILE_FLAGS "-Wall -Wimplicit-fallthrough -fno-rtti -fPIC"
)

target_link_libraries(KINT ${CMAKE_CURRENT_SOURCE_DIR}/../boolector/build/lib/libboolector.a)
target_link_libraries(KINT ${CMAKE_CURRENT_SOURCE_DIR}/../boolector/deps/install/lib/liblgl.a)
target_link_libraries(KINT ${CMAKE_CURRENT_SOURCE_DIR}/../boolector/deps/install/lib/libbtor2parser.a)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(KINT PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
