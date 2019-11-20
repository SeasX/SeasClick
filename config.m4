dnl $Id$
dnl config.m4 for extension SeasClick

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(SeasClick, for SeasClick support,
dnl Make sure that the comment is aligned:
dnl [  --with-SeasClick             Include SeasClick support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(SeasClick, whether to enable SeasClick support,
Make sure that the comment is aligned:
[  --enable-SeasClick           Enable SeasClick support])
PHP_ARG_ENABLE(swoole, enable swoole support,
[  --enable-swoole            Use swoole?], no, no)

if test "$PHP_SEASCLICK" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-SeasClick -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/SeasClick.h"  # you most likely want to change this
  dnl if test -r $PHP_SEASCLICK/$SEARCH_FOR; then # path given as parameter
  dnl   SEASCLICK_DIR=$PHP_SEASCLICK
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for SeasClick files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SEASCLICK_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SEASCLICK_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the SeasClick distribution])
  dnl fi

  dnl # --with-SeasClick -> add include path
  dnl PHP_ADD_INCLUDE($SEASCLICK_DIR/include)

  dnl # --with-SeasClick -> check for lib and symbol presence
  dnl LIBNAME=SeasClick # you may want to change this
  dnl LIBSYMBOL=SeasClick # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SEASCLICK_DIR/$PHP_LIBDIR, SEASCLICK_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SEASCLICKLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong SeasClick lib version or lib not found])
  dnl ],[
  dnl   -L$SEASCLICK_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SEASCLICK_SHARED_LIBADD)

  if test "$PHP_SWOOLE" = "yes"; then
      AC_DEFINE(USE_SWOOLE, 1, [enable swoole support])
      CXXFLAGS="$CXXFLAGS -DUSE_SWOOLE"
  fi
  PHP_REQUIRE_CXX()
  PHP_SUBST(SEASCLICK_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, SEASCLICK_SHARED_LIBADD)
  CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Wno-deprecated -Wno-deprecated-declarations -std=c++17"
  SeasClick_source_file="SeasClick.cpp \
        typesToPhp.cpp \
        lib/clickhouse-cpp/clickhouse/base/coded.cpp \
        lib/clickhouse-cpp/clickhouse/base/compressed.cpp \
        lib/clickhouse-cpp/clickhouse/base/input.cpp \
        lib/clickhouse-cpp/clickhouse/base/output.cpp \
        lib/clickhouse-cpp/clickhouse/base/platform.cpp \
        lib/clickhouse-cpp/clickhouse/base/socket.cpp \
        lib/clickhouse-cpp/clickhouse/columns/array.cpp \
        lib/clickhouse-cpp/clickhouse/columns/date.cpp \
        lib/clickhouse-cpp/clickhouse/columns/decimal.cpp \
        lib/clickhouse-cpp/clickhouse/columns/enum.cpp \
        lib/clickhouse-cpp/clickhouse/columns/factory.cpp \
        lib/clickhouse-cpp/clickhouse/columns/nullable.cpp \
        lib/clickhouse-cpp/clickhouse/columns/numeric.cpp \
        lib/clickhouse-cpp/clickhouse/columns/string.cpp \
        lib/clickhouse-cpp/clickhouse/columns/tuple.cpp \
        lib/clickhouse-cpp/clickhouse/columns/uuid.cpp \
        lib/clickhouse-cpp/clickhouse/types/type_parser.cpp \
        lib/clickhouse-cpp/clickhouse/types/types.cpp \
        lib/clickhouse-cpp/contrib/cityhash/city.cc \
        lib/clickhouse-cpp/contrib/lz4/lz4.c \
        lib/clickhouse-cpp/contrib/lz4/lz4hc.c \
        lib/clickhouse-cpp/contrib/gtest/gtest-all.cc \
        lib/clickhouse-cpp/clickhouse/block.cpp \
        lib/clickhouse-cpp/clickhouse/client.cpp \
        lib/clickhouse-cpp/clickhouse/query.cpp"
  SeasClick_header_file="lib/clickhouse-cpp/contrib"

  THIS_DIR=`dirname $0`
  PHP_ADD_INCLUDE($THIS_DIR/lib/clickhouse-cpp/contrib)
  
  PHP_NEW_EXTENSION(SeasClick, $SeasClick_source_file, $ext_shared,,-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/contrib)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/contrib/cityhash)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/contrib/gtest)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/contrib/lz4)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/clickhouse)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/clickhouse/base)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/clickhouse/types)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp/clickhouse/columns)
  PHP_ADD_BUILD_DIR($ext_builddir/lib/clickhouse-cpp)
fi
