#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------

#
# The std types helper requires C99 at minimum for C.
#
if (__ulang_std_types_isloaded)
    return ()
endif ()
set (__ulang_std_types_isloaded YES)

include (CheckIncludeFiles)
include (CheckTypeSize)


# Auto-detect and define the standard integer types.
#
# As this macro generates an output C/${_lang} header, you must
# additionally add this header as a dependency of your
# project, else parallel builds may not function.
#
# The commands below add the header to the `project' target:
#
#    add_custom_target (project_stdtypes_h DEPENDS ${HEADER})
#    add_dependencies (project project_stdtypes_h)
#
#
macro (enable_std_types _output _moduledir _lang)
    check_include_files (inttypes.h  HAVE_INTTYPES_H)
    check_include_files (stdint.h    HAVE_STDINT_H)
    check_include_files (sys/types.h HAVE_SYS_TYPES_H)
    check_include_files (stdlib.h    HAVE_STDLIB_H)
    check_include_files (stddef.h    HAVE_STDDEF_H)

    check_type_size (__int8  __INT8 LANGUAGE ${_lang})
    check_type_size (int8_t  INT8_T LANGUAGE ${_lang})
    check_type_size (uint8_t UINT8_T LANGUAGE ${_lang})

    check_type_size (__int16  __INT16 LANGUAGE ${_lang})
    check_type_size (int16_t  INT16_T LANGUAGE ${_lang})
    check_type_size (uint16_t UINT16_T LANGUAGE ${_lang})

    check_type_size (__int32  __INT32 LANGUAGE ${_lang})
    check_type_size (int32_t  INT32_T LANGUAGE ${_lang})
    check_type_size (uint32_t UINT32_T LANGUAGE ${_lang})

    check_type_size (__int64  __INT64 LANGUAGE ${_lang})
    check_type_size (int64_t  INT64_T LANGUAGE ${_lang})
    check_type_size (uint64_t UINT64_T LANGUAGE ${_lang})

    check_type_size (ssize_t  SSIZE_T LANGUAGE ${_lang})
    check_type_size (size_t   SIZE_T  LANGUAGE ${_lang})

    check_type_size (short       SHORT_SIZE     BUILTIN_TYPES_ONLY
                     LANGUAGE ${_lang})
    check_type_size (int         INT_SIZE       BUILTIN_TYPES_ONLY
                     LANGUAGE ${_lang})
    check_type_size (long        LONG_SIZE      BUILTIN_TYPES_ONLY
                     LANGUAGE ${_lang})
    check_type_size ("long long" LONG_LONG_SIZE BUILTIN_TYPES_ONLY
                     LANGUAGE ${_lang})

    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${_moduledir}/stdtypes.h.in
                    ${_output} @ONLY)
endmacro ()

# vim: set ts=2 sw=2 expandtab :