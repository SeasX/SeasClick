/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef __cplusplus
#define __STDC_FORMAT_MACROS
#endif

#ifndef PHP_SEASCLICK_H
#define PHP_SEASCLICK_H

extern zend_module_entry SeasClick_module_entry;
#define phpext_SeasClick_ptr &SeasClick_module_entry

#define PHP_SEASCLICK_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_SEASCLICK_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SEASCLICK_API __attribute__ ((visibility("default")))
#else
#	define PHP_SEASCLICK_API
#endif

extern "C" {
#ifdef ZTS
#include "TSRM.h"
#endif
}

typedef unsigned long ulong_t;
/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(SeasClick)
	zend_ulong  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(SeasClick)
*/

/* Always refer to the globals in your function as SEASCLICK_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SEASCLICK_G(v) TSRMG(SeasClick_globals_id, zend_SeasClick_globals *, v)
#else
#define SEASCLICK_G(v) (SeasClick_globals.v)
#endif

#define SEASCLICK_RES_NAME                        "SeasClick"

#endif	/* PHP_SEASCLICK_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
