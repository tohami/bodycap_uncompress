cmake_minimum_required(VERSION 3.4.1)  # for example


add_library( Uncompress

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ../ios/Classes/lib_uncompress.c
             ../ios/Classes/lib_uncompress.h
             ../ios/Classes/lib_bitStream.c
             ../ios/Classes/lib_bitStream.h
             ../ios/Classes/lib_compress_defines.h
             ../ios/Classes/nrf_log.h
             ../ios/Classes/project.h
              )

find_library( log-lib log )
target_link_libraries(Uncompress ${log-lib} )