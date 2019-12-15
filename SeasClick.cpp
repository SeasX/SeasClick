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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C" {
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "php7_wrapper.h"
}

#include "php_SeasClick.h"

#include "lib/clickhouse-cpp/clickhouse/client.h"
#include "lib/clickhouse-cpp/clickhouse/error_codes.h"
#include "lib/clickhouse-cpp/clickhouse/types/type_parser.h"
#include "typesToPhp.hpp"
#include <iostream>
#include <map>
#include <sstream>

using namespace clickhouse;
using namespace std;

zend_class_entry *SeasClick_ce;
map<int, Client*> clientMap;

#ifdef COMPILE_DL_SEASCLICK
extern "C" {
    ZEND_GET_MODULE(SeasClick)
}
#endif

PHP_FUNCTION(SeasClick_version)
{
    SC_RETURN_STRINGL(PHP_SEASCLICK_VERSION, strlen(PHP_SEASCLICK_VERSION));
}

static PHP_METHOD(SEASCLICK_RES_NAME, __construct);
static PHP_METHOD(SEASCLICK_RES_NAME, __destruct);
static PHP_METHOD(SEASCLICK_RES_NAME, select);
static PHP_METHOD(SEASCLICK_RES_NAME, insert);
static PHP_METHOD(SEASCLICK_RES_NAME, execute);

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_construct, 0, 0, 1)
ZEND_ARG_INFO(0, connectParames)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_select, 0, 0, 3)
ZEND_ARG_INFO(0, sql)
ZEND_ARG_INFO(0, params)
ZEND_ARG_INFO(0, fetch_mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_insert, 0, 0, 3)
ZEND_ARG_INFO(0, table)
ZEND_ARG_INFO(0, columns)
ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_execute, 0, 0, 2)
ZEND_ARG_INFO(0, sql)
ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

/* {{{ SeasClick_functions[] */
const zend_function_entry SeasClick_functions[] =
{
    PHP_FE(SeasClick_version,	NULL)
    PHP_FE_END
};
/* }}} */

const zend_function_entry SeasClick_methods[] =
{
    PHP_ME(SEASCLICK_RES_NAME, __construct,   SeasCilck_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(SEASCLICK_RES_NAME, __destruct,    NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(SEASCLICK_RES_NAME, select,   SeasCilck_select, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, insert,   SeasCilck_insert, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, execute,   SeasCilck_execute, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

#define REGISTER_SC_CLASS_CONST_LONG(const_name, value) \
	zend_declare_class_constant_long(SeasClick_ce, const_name, sizeof(const_name)-1, (zend_long)value);

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(SeasClick)
{
    zend_class_entry SeasClick;
    INIT_CLASS_ENTRY(SeasClick, SEASCLICK_RES_NAME, SeasClick_methods);
#if PHP_VERSION_ID >= 70000
    SeasClick_ce = zend_register_internal_class_ex(&SeasClick, NULL);
#else
    SeasClick_ce = zend_register_internal_class_ex(&SeasClick, NULL, NULL TSRMLS_CC);
#endif
    zend_declare_property_stringl(SeasClick_ce, "host", strlen("host"), "127.0.0.1", sizeof("127.0.0.1") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(SeasClick_ce, "port", strlen("port"), 9000, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_stringl(SeasClick_ce, "database", strlen("database"), "default", sizeof("default") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "user", strlen("user"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "passwd", strlen("passwd"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_bool(SeasClick_ce, "compression", strlen("compression"), false, ZEND_ACC_PROTECTED TSRMLS_CC);

    REGISTER_SC_CLASS_CONST_LONG("FETCH_ONE", (zend_long)SC_FETCH_ONE);
    REGISTER_SC_CLASS_CONST_LONG("FETCH_KEY_PAIR", (zend_long)SC_FETCH_KEY_PAIR);
    REGISTER_SC_CLASS_CONST_LONG("DATE_AS_STRINGS", (zend_long)SC_FETCH_DATE_AS_STRINGS);
    REGISTER_SC_CLASS_CONST_LONG("FETCH_COLUMN", (zend_long)SC_FETCH_COLUMN);

    SeasClick_ce->ce_flags = ZEND_ACC_IMPLICIT_PUBLIC;
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(SeasClick)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "SeasClick support", "enabled");
    php_info_print_table_row(2, "Version", PHP_SEASCLICK_VERSION);
    php_info_print_table_row(2, "Author", "SeasX Group[email: ahhhh.wang@gmail.com]");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ SeasClick_module_entry
 */
zend_module_entry SeasClick_module_entry =
{
    STANDARD_MODULE_HEADER,
    SEASCLICK_RES_NAME,
    SeasClick_functions,
    PHP_MINIT(SeasClick),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(SeasClick),
    PHP_SEASCLICK_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* {{{ proto object __construct(array connectParames)
 */
PHP_METHOD(SEASCLICK_RES_NAME, __construct)
{
    zval *connectParames;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &connectParames) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY(connectParames)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif

    HashTable *_ht = Z_ARRVAL_P(connectParames);
    zval *value;

    zval *this_obj;
    this_obj = getThis();
    if (php_array_get_value(_ht, "host", value))
    {
        convert_to_string(value);
        zend_update_property_string(SeasClick_ce, this_obj, "host", sizeof("host") - 1, Z_STRVAL_P(value) TSRMLS_CC);
    }

    if (php_array_get_value(_ht, "port", value))
    {
        convert_to_long(value);
        zend_update_property_long(SeasClick_ce, this_obj, "port", sizeof("port") - 1, Z_LVAL_P(value) TSRMLS_CC);
    }

    if (php_array_get_value(_ht, "compression", value))
    {
        convert_to_boolean(value);
        zend_update_property_bool(SeasClick_ce, this_obj, "compression", sizeof("compression") - 1, Z_LVAL_P(value) TSRMLS_CC);
    }

    zval *host = sc_zend_read_property(SeasClick_ce, this_obj, "host", sizeof("host") - 1, 0);
    zval *port = sc_zend_read_property(SeasClick_ce, this_obj, "port", sizeof("port") - 1, 0);
    zval *compression = sc_zend_read_property(SeasClick_ce, this_obj, "compression", sizeof("compression") - 1, 0);

    ClientOptions Options = ClientOptions()
                            .SetHost(Z_STRVAL_P(host))
                            .SetPort(Z_LVAL_P(port))
                            .SetPingBeforeQuery(false);
    if (Z_TYPE_P(compression) == IS_TRUE)
    {
        Options = Options.SetCompressionMethod(CompressionMethod::LZ4);
    }

    if (php_array_get_value(_ht, "database", value))
    {
        convert_to_string(value);
        zend_update_property_string(SeasClick_ce, this_obj, "database", sizeof("database") - 1, Z_STRVAL_P(value) TSRMLS_CC);
        Options = Options.SetDefaultDatabase(Z_STRVAL_P(value));
    }

    if (php_array_get_value(_ht, "user", value))
    {
        convert_to_string(value);
        zend_update_property_string(SeasClick_ce, this_obj, "user", sizeof("user") - 1, Z_STRVAL_P(value) TSRMLS_CC);
        Options = Options.SetUser(Z_STRVAL_P(value));
    }

    if (php_array_get_value(_ht, "passwd", value))
    {
        convert_to_string(value);
        zend_update_property_string(SeasClick_ce, this_obj, "passwd", sizeof("passwd") - 1, Z_STRVAL_P(value) TSRMLS_CC);
        Options = Options.SetPassword(Z_STRVAL_P(value));
    }

    try
    {
        Client *client = new Client(Options);
        int key = Z_OBJ_HANDLE(*this_obj);

        clientMap.insert(std::pair<int, Client*>(key, client));

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }

    RETURN_TRUE;
}
/* }}} */

void getInsertSql(string *sql, char *table_name, zval *columns)
{
    zval *pzval;
    char *key;
    uint32_t keylen;
    int keytype;

    std::stringstream fields_section;

    HashTable *columns_ht = Z_ARRVAL_P(columns);
    size_t count = zend_hash_num_elements(columns_ht);
    size_t index = 0;

    SC_HASHTABLE_FOREACH_START2(columns_ht, key, keylen, keytype, pzval)
    {
        convert_to_string(pzval);
        if (index >= (count - 1))
        {
            fields_section << (string)Z_STRVAL_P(pzval);
        }
        else
        {
            fields_section << (string)Z_STRVAL_P(pzval) << ",";
        }
        index++;
    }
    SC_HASHTABLE_FOREACH_END();
    *sql = "INSERT INTO " + (string)table_name + " ( " + fields_section.str() + " ) VALUES";
}

/* {{{ proto array select(string sql, array params, int mode)
 */
PHP_METHOD(SEASCLICK_RES_NAME, select)
{
    char *sql = NULL;
    size_t l_sql = 0;
    zval* params = NULL;
    zend_long fetch_mode = 0;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zl", &sql, &l_sql, &params, &fetch_mode) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(1, 3)
    Z_PARAM_STRING(sql, l_sql)
    Z_PARAM_OPTIONAL
    Z_PARAM_ARRAY(params)
    Z_PARAM_LONG(fetch_mode)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif
    try
    {
        string sql_s = (string)sql;
        if (ZEND_NUM_ARGS() > 1 && params != NULL)
        {
            if (Z_TYPE_P(params) != IS_ARRAY)
            {
                throw std::runtime_error("The second argument to the select function must be an array");
            }

            HashTable *params_ht = Z_ARRVAL_P(params);
            zval *pzval;
            char *str_key;
            uint32_t str_keylen;
            int keytype;

            SC_HASHTABLE_FOREACH_START2(params_ht, str_key, str_keylen, keytype, pzval)
            {
                convert_to_string(pzval);
                sql_s.replace(sql_s.find("{" + (string)str_key + "}"), str_keylen + 2, (string)Z_STRVAL_P(pzval));
            }
            SC_HASHTABLE_FOREACH_END();
        }

        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        if (!(fetch_mode & SC_FETCH_ONE)) {
            array_init(return_value);
        }

        client->Select(sql_s, [return_value, fetch_mode](const Block &block) {
            if (fetch_mode & SC_FETCH_ONE) {
                if (block.GetRowCount() > 0 && block.GetColumnCount() > 0) {
                    convertToZval(return_value, block[0], 0, "", 0, fetch_mode);
                }
                return;
            }

            zval *return_tmp;
            for (size_t row = 0; row < block.GetRowCount(); ++row)
            {
                if (fetch_mode & SC_FETCH_KEY_PAIR) {
                    if (block.GetColumnCount() < 2) {
                        throw std::runtime_error("Key pair mode requires at least 2 columns to be present");
                    }
                    zval *col1, *col2;
                    SC_MAKE_STD_ZVAL(col1);
                    SC_MAKE_STD_ZVAL(col2);

                    convertToZval(col1, block[0], row, "", 0, fetch_mode|SC_FETCH_ONE);
                    convertToZval(col2, block[1], row, "", 0, fetch_mode|SC_FETCH_ONE);

                    if (Z_TYPE_P(col1) == IS_LONG) {
                         zend_hash_index_update(Z_ARRVAL_P(return_value), Z_LVAL_P(col1), col2);
                    } else {
                        convert_to_string(col1);
                        zend_symtable_update(Z_ARRVAL_P(return_value), Z_STR_P(col1), col2);
                    }
                    zval_ptr_dtor(col1);
                    continue;
                }

                SC_MAKE_STD_ZVAL(return_tmp);
                if (!(fetch_mode & SC_FETCH_COLUMN)) {
                    array_init(return_tmp);
                }

                for (size_t column = 0; column < block.GetColumnCount(); ++column)
                {
                    string column_name = block.GetColumnName(column);
                    if (fetch_mode & SC_FETCH_COLUMN) {
                        convertToZval(return_tmp, block[0], row, "", 0, fetch_mode|SC_FETCH_ONE);
                        break;
                    } else {
                        convertToZval(return_tmp, block[column], row, column_name, 0, fetch_mode);
                    }
                }
                add_next_index_zval(return_value, return_tmp);
            }
        });
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
}
/* }}} */

/* {{{ proto array insert(string table, array columns, array values)
 */
PHP_METHOD(SEASCLICK_RES_NAME, insert)
{
    char *table = NULL;
    size_t l_table = 0;
    zval *columns;
    zval *values;

    string sql;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz", &table, &l_table, &columns, &values) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_STRING(table, l_table)
    Z_PARAM_ARRAY(columns)
    Z_PARAM_ARRAY(values)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif

    try
    {
        HashTable *columns_ht = Z_ARRVAL_P(columns);
        HashTable *values_ht = Z_ARRVAL_P(values);
        size_t columns_count = zend_hash_num_elements(columns_ht);

        zval *return_should;
        SC_MAKE_STD_ZVAL(return_should);
        array_init(return_should);

        zval *fzval;
        zval *pzval;
        char *str_key;
        uint32_t str_keylen;
        int keytype;

        zval *return_tmp;
        for(size_t i = 0; i < columns_count; i++)
        {
            SC_MAKE_STD_ZVAL(return_tmp);
            array_init(return_tmp);

            SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, pzval)
            {
                if (Z_TYPE_P(pzval) != IS_ARRAY)
                {
                    throw std::runtime_error("The insert function needs to pass in a two-dimensional array");
                }
                fzval = sc_zend_hash_index_find(Z_ARRVAL_P(pzval), i);
                if (NULL == fzval)
                {
                    throw std::runtime_error("The number of parameters inserted per line is inconsistent");
                }
                sc_zval_add_ref(fzval);
                add_next_index_zval(return_tmp, fzval);
            }
            SC_HASHTABLE_FOREACH_END();

            add_next_index_zval(return_should, return_tmp);
        }

        getInsertSql(&sql, table, columns);
        Block blockQuery;

        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        client->InsertQuery(sql, [&blockQuery](const Block& block)
        {
            blockQuery = block;
        }
                           );

        Block blockInsert;
        size_t index = 0;

        SC_HASHTABLE_FOREACH_START2(Z_ARRVAL_P(return_should), str_key, str_keylen, keytype, pzval)
        {
            zvalToBlock(blockInsert, blockQuery, index, pzval);
            index++;
        }
        SC_HASHTABLE_FOREACH_END();

        client->InsertData(blockInsert);
        sc_zval_ptr_dtor(&return_should);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool execute(string sql, array params)
 */
PHP_METHOD(SEASCLICK_RES_NAME, execute)
{
    char *sql = NULL;
    size_t l_sql = 0;
    zval* params = NULL;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &sql, &l_sql, &params) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STRING(sql, l_sql)
    Z_PARAM_OPTIONAL
    Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif

    try
    {
        string sql_s = (string)sql;
        if (ZEND_NUM_ARGS() > 1 && params != NULL)
        {
            if (Z_TYPE_P(params) != IS_ARRAY)
            {
                throw std::runtime_error("The second argument to the select function must be an array");
            }

            HashTable *params_ht = Z_ARRVAL_P(params);
            zval *pzval;
            char *str_key;
            uint32_t str_keylen;
            int keytype;

            SC_HASHTABLE_FOREACH_START2(params_ht, str_key, str_keylen, keytype, pzval)
            {
                convert_to_string(pzval);
                sql_s.replace(sql_s.find("{" + (string)str_key + "}"), str_keylen + 2, (string)Z_STRVAL_P(pzval));
            }
            SC_HASHTABLE_FOREACH_END();
        }

        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);
        client->Execute(sql_s);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto array __destruct()
 */
PHP_METHOD(SEASCLICK_RES_NAME, __destruct)
{
    try
    {
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);
        delete client;
        clientMap.erase(key);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
    RETURN_TRUE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
