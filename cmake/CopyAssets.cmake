# Copy Assets directory to destination while excluding Assets/Logs
if(NOT DEFINED SRC_DIR OR NOT DEFINED DST_DIR)
    message(FATAL_ERROR "SRC_DIR and DST_DIR must be defined")
endif()

file(MAKE_DIRECTORY "${DST_DIR}")
file(GLOB_RECURSE asset_files RELATIVE "${SRC_DIR}" "${SRC_DIR}/*")

foreach(rel_path IN LISTS asset_files)
    if(rel_path MATCHES "^Logs(/|$)")
        continue()
    endif()
    set(src_file "${SRC_DIR}/${rel_path}")
    set(dst_file "${DST_DIR}/${rel_path}")
    get_filename_component(dst_parent "${dst_file}" DIRECTORY)
    file(MAKE_DIRECTORY "${dst_parent}")
    configure_file("${src_file}" "${dst_file}" COPYONLY)
endforeach()
