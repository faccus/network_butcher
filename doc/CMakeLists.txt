find_package(Doxygen
             OPTIONAL_COMPONENTS dot mscgen dia)

if( DOXYGEN_FOUND )
    set(DOXYGEN_EXCLUDE_PATTERNS "*/_deps/*")
    doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}
            COMMENT "Generate man pages"
    )

elseif( BUILD_DOC )
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()