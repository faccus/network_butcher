find_package(Doxygen
        OPTIONAL_COMPONENTS dot mscgen dia) # doxygen, graphviz

if (DOXYGEN_FOUND)
    set(Doxygen_FIND_QUIETLY TRUE)

    set(DOXYGEN_INHERIT_DOCS NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_PRIV_STATIC YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)
    set(DOXYGEN_RECURSIVE YES)

    SET(DOXYGEN_DIRECTORY_GRAPH NO)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    SET(DOXYGEN_GROUP_GRAPHS NO)

    set(DOXYGEN_OUTPUT_DIRECTORY doc)
    set(DOXYGEN_TEMPLATE_RELATIONS YES)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_HTML_FILE_EXTENSION .html)
    set(DOXYGEN_INTERACTIVE_SVG YES)
    set(DOXYGEN_UML_LIMIT_NUM_FIELDS 100)
    set(DOXYGEN_NUM_PROC_THREADS 0)
    set(DOXYGEN_TIMESTAMP DATETIME)

    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)

    doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/README.md
            COMMENT "Generate doc pages"
    )
elseif ()
    message(STATUS "Missing doxygen: I will not be able to produce documentation files!")
endif ()