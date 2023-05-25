find_package(Doxygen
        OPTIONAL_COMPONENTS dot mscgen dia) # doxygen, graphviz

if (DOXYGEN_FOUND)
    set(Doxygen_FIND_QUIETLY TRUE)

    set(DOXYGEN_EXCLUDE_PATTERNS */onnx_proto/* */test_data/* */_deps/* */google_tests/* */dep/*)
    set(DOXYGEN_INHERIT_DOCS NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_PRIV_STATIC YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)

    # Collaboration graphs
    set(DOXYGEN_OUTPUT_DIRECTORY doc)
    set(DOXYGEN_TEMPLATE_RELATIONS YES)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_HTML_FILE_EXTENSION .xhtml)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_INTERACTIVE_SVG YES)
    set(DOXYGEN_UML_LIMIT_NUM_FIELDS 100)

    doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}
            COMMENT "Generate doc pages"
    )
    
    
    set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                   ${DOXYFILE} @ONLY)
    
    add_custom_target(
      doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation")
elseif ()
    message(STATUS "Missing doxygen: I will not be able to produce documentation files!")
endif ()
