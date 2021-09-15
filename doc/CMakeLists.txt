find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)

if( DOXYGEN_FOUND )

    set(DOXYGEN_EXCLUDE_PATTERNS */Onnx_model/* */_deps/*)


    # Collaboration graphs
    set(DOXYGEN_INHERIT_DOCS NO)
    set(DOXYGEN_TEMPLATE_RELATIONS YES)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_HTML_FILE_EXTENSION .xhtml)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_INTERACTIVE_SVG YES)

    doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}
            COMMENT "Generate man pages"
    )

elseif( BUILD_DOC )
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()