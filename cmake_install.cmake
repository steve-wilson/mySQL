# Install script for directory: /home/steve/mysql-server/mysql-5.6

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local/mysql")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Info")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/docs" TYPE FILE OPTIONAL FILES "/home/steve/mysql-server/mysql-5.6/Docs/mysql.info")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Info")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE OPTIONAL FILES
    "/home/steve/mysql-server/mysql-5.6/COPYING"
    "/home/steve/mysql-server/mysql-5.6/LICENSE.mysql"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "/home/steve/mysql-server/mysql-5.6/README")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/docs" TYPE FILE FILES
    "/home/steve/mysql-server/mysql-5.6/Docs/INFO_SRC"
    "/home/steve/mysql-server/mysql-5.6/Docs/INFO_BIN"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "/home/steve/mysql-server/mysql-5.6/Docs/INSTALL-BINARY")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Documentation")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/docs" TYPE DIRECTORY FILES "/home/steve/mysql-server/mysql-5.6/Docs/" REGEX "/INSTALL\\-BINARY$" EXCLUDE REGEX "/Makefile\\.[^/]*$" EXCLUDE REGEX "/glibc[^/]*$" EXCLUDE REGEX "/linuxthreads\\.txt$" EXCLUDE REGEX "/myisam\\.txt$" EXCLUDE REGEX "/mysql\\.info$" EXCLUDE REGEX "/sp\\-imp\\-spec\\.txt$" EXCLUDE)
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Documentation")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/steve/mysql-server/mysql-5.6/zlib/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/extra/yassl/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/extra/yassl/taocrypt/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/cmd-line-utils/libedit/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/libevent/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/example/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/blackhole/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/innobase/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/ndb/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/csv/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/perfschema/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/myisammrg/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/federated/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/heap/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/archive/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/storage/myisam/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/semisync/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/auth/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/innodb_memcached/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/daemon_example/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/audit_null/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/password_validation/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/plugin/fulltext/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/include/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/dbug/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/strings/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/vio/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/regex/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/mysys/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/mysys_ssl/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/libmysql/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/unittest/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/unittest/examples/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/unittest/mytap/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/unittest/mytap/t/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/unittest/gunit/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/extra/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/tests/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/client/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/sql/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/sql/share/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/libservices/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/libmysqld/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/libmysqld/examples/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/mysql-test/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/mysql-test/lib/My/SafeProcess/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/support-files/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/scripts/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/sql-bench/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/man/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/packaging/rpm-uln/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/packaging/WiX/cmake_install.cmake")
  INCLUDE("/home/steve/mysql-server/mysql-5.6/packaging/solaris/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/steve/mysql-server/mysql-5.6/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/steve/mysql-server/mysql-5.6/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
