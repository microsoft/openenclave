# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

function(sgx_enclave_configure_target TARGET)
    # NOTE: These three work for CMake 3.13+, but Azure DevOps currently has
    # 3.12 installed:
    #
    # target_link_options(${TARGET} BEFORE PRIVATE "/NODEFAULTLIB")
    # target_link_options(${TARGET} BEFORE PRIVATE "/NOENTRY")
    # target_link_options(${TARGET} BEFORE PRIVATE "/MANIFEST:NO")
    #
    # Workaround follows:
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/NODEFAULTLIB /NOENTRY /MANIFEST:NO")

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${SGX_SDK_SIGN_TOOL} sign
            -key ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}_private.pem
            -enclave ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>/${TARGET}.dll
            -out ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>/${TARGET}.signed.dll
            -config ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.config.xml)
endfunction()
