atlas_add_web_library(
    PREFIX plainsong
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    TARGET plainsong-web
    SOURCES
        index.html
        index.js
        )
