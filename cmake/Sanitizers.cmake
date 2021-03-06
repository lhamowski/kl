# Sanitizers
#  Included, adds new build or configuration types:
#    - ASan - AddressSanitizer
#    - UBSan - UndefinedBehaviorSanitizer
#    - TSan - ThreadSanitizer
#    - MSan - MemorySanitizer (Clang only)
#

# This only works for GCC or Clang
if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
   NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    message(STATUS "No sanitizing build types for compiler: ${CMAKE_CXX_COMPILER_ID}")
    return()
endif()

include(DefineBuildType)

kl_define_build_type(ASan
    COMPILER_FLAGS "-O1 -g -fsanitize=address -fno-omit-frame-pointer -DNDEBUG"
)
kl_define_build_type(TSan
    COMPILER_FLAGS "-O1 -g -fsanitize=thread -DNDEBUG"
)
kl_define_build_type(UBSan
    COMPILER_FLAGS "-O1 -g -fsanitize=undefined -fno-omit-frame-pointer -DNDEBUG"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    kl_define_build_type(MSan
        COMPILER_FLAGS "-O2 -g -fsanitize=memory -fno-omit-frame-pointer -DNDEBUG"
    )
endif()
