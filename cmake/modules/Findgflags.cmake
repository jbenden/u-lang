#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------
if (__ulang_findgflags_isloaded)
  return ()
endif ()
set (__ulang_findgflags_isloaded YES)

set(HAVE_GFLAGS ON)

# vim: set ts=2 sw=2 expandtab :