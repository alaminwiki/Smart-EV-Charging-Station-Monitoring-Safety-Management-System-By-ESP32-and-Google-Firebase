# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/alami/esp/v5.5.2/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "C:/Users/alami/esp/v5.5.2/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader"
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix"
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/tmp"
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/src/bootloader-stamp"
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/src"
  "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/IOT Lab Practice By Al Amin/Firebase-ESP-IDF-IY-main___ESP03/Firebase-ESP-IDF-IY-main/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
