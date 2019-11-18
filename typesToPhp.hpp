/*
  +----------------------------------------------------------------------+
  | SeasClick                                                            |
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
  | Author:  SeasX Group <ahhhh.wang@gmail.com>                          |
  +----------------------------------------------------------------------+
*/
using namespace clickhouse;
using namespace std;

ColumnRef createColumn(TypeRef type);

ColumnRef insertColumn(TypeRef type, zval *value_zval);

std::string int128_to_string(Int128 value, size_t scale);

void convertToZval(zval *arr, const ColumnRef& columnRef, int row, string column_name, int8_t is_array);

void zvalToBlock(Block& blockDes, Block& blockSrc, zend_ulong num_key, zval *value_zval);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
