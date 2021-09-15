if( BUILD_DOC )
    if( BUILD_DOC_GRAPH )
        find_package(Doxygen
                REQUIRED dot
                OPTIONAL_COMPONENTS mscgen dia)
    else()
        find_package(Doxygen
                OPTIONAL_COMPONENTS dot mscgen dia)
    endif()
    set(DOXYGEN_EXCLUDE_PATTERNS */Onnx_model/* */_deps/*)
    set(DOXYGEN_INHERIT_DOCS NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_PRIV_STATIC YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)


    if ( DOXYGEN_FOUND )

        # Collaboration graphs
        set(DOXYGEN_TEMPLATE_RELATIONS YES)
        set(DOXYGEN_UML_LOOK YES)
        set(DOXYGEN_HTML_FILE_EXTENSION .xhtml)
        set(DOXYGEN_DOT_IMAGE_FORMAT svg)
        set(DOXYGEN_INTERACTIVE_SVG YES)
        set(DOXYGEN_UML_LIMIT_NUM_FIELDS 100)

        doxygen_add_docs(
                doxygen
                ${PROJECT_SOURCE_DIR}
                COMMENT "Generate man pages"
        )
    elseif ()
        message(STATUS "Missing doxygen: I will not be able to produce documentation files!")
    endif()
endif()