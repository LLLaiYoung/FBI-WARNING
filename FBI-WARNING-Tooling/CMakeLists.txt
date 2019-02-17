set(LLVM_LINK_COMPONENTS
    Support
)

add_clang_executable(FBI-WARNING-Tooling
	FBI-WARNING.cpp
	FWPluginComm.hpp
        FWPluginPropertyCheck.hpp
)

target_link_libraries(FBI-WARNING-Tooling
    PRIVATE
    clangAST
    clangBasic
    clangDriver
    clangFormat
    clangLex
    clangParse
    clangSema
    clangFrontend
    clangTooling
    clangToolingCore
    clangRewrite
    clangRewriteFrontend
)

if (UNIX)
    set(CLANGXX__LING_OR_COPY create_symlink)
else()
    set(CLANGXX_LINK_OR_COPY copy)
endif()
