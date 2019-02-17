add_llvm_library(FBI-WARNING MODULE 
	FBI-WARNING.cpp
	FWPluginComm.hpp
        FWPluginPropertyCheck.hpp
	PLUGIN_TOOL
	clang
)

if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(FBI-WARNING PRIVATE
    clangAST
    clangBasic
    clangFrontend
    LLVMSupport
    )
endif()
