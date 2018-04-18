#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------
if (__ulang_read_version_isloaded)
    return ()
endif ()
set (__ulang_read_version_isloaded YES)

macro (read_version _include _prefix)
#
# Read our version
#
file (STRINGS ${_include} the_version_str
      REGEX "^#[\t ]*define[\t ]+${_prefix}_(MAJOR|MINOR|MICRO)_VERSION[\t ]+[0-9]+$")
set (PROJECT_VERSION "")
foreach (VPART MAJOR MINOR MICRO)
  foreach (VLINE ${the_version_str})
    if (VLINE MATCHES "^#[\t ]*define[\t ]+${_prefix}_${VPART}_VERSION[\t ]+([0-9]+)$")
      set (${_prefix}_VERSION_PART "${CMAKE_MATCH_1}")
      set (${_prefix}_${VPART}_VERSION "${${_prefix}_VERSION_PART}")
      if (NOT PROJECT_VERSION STREQUAL "")
        set (PROJECT_VERSION "${PROJECT_VERSION}.${${_prefix}_VERSION_PART}")
      else ()
        set (PROJECT_VERSION "${${_prefix}_VERSION_PART}")
      endif ()
    endif ()
  endforeach ()
endforeach ()
message (STATUS "This is ${PROJECT_NAME} version ${PROJECT_VERSION}")
endmacro ()

# vim: set ts=2 sw=2 expandtab :