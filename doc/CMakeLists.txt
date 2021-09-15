if( BUILD_DOC )

    if ( DOXYGEN_FOUND )

        set(DOXYGEN_EXCLUDE_PATTERNS */Onnx_model/* */_deps/*)
        set(DOXYGEN_INHERIT_DOCS NO)

        if( DOXYGEN_HAVE_DOT )
            # Collaboration graphs
            set(DOXYGEN_TEMPLATE_RELATIONS YES)
            set(DOXYGEN_UML_LOOK YES)
            set(DOXYGEN_HTML_FILE_EXTENSION .xhtml)
            set(DOXYGEN_DOT_IMAGE_FORMAT svg)
            set(DOXYGEN_INTERACTIVE_SVG YES)

        elseif()
            message(STATUS "Missing dot: I will not be able to produce graphs!")
        endif()

        doxygen_add_docs(
                doxygen
                ${PROJECT_SOURCE_DIR}
                COMMENT "Generate man pages"
        )
    elseif ()
        message(STATUS "Missing doxygen: I will not be able to produce documentation files!")
    endif()
endif()