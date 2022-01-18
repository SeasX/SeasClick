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
map<int, Block> clientInsertBlack;

#ifdef COMPILE_DL_SEASCLICK
extern "C" {
    ZEND_GET_MODULE(SeasClick)
}
#endif

// PHP_FUNCTION(SeasClick_version)
// {
//     SC_RETURN_STRINGL(PHP_SEASCLICK_VERSION, strlen(PHP_SEASCLICK_VERSION));
// }

static PHP_METHOD(SEASCLICK_RES_NAME, __construct);
static PHP_METHOD(SEASCLICK_RES_NAME, __destruct);
static PHP_METHOD(SEASCLICK_RES_NAME, select);
static PHP_METHOD(SEASCLICK_RES_NAME, insert);
static PHP_METHOD(SEASCLICK_RES_NAME, writeStart);
static PHP_METHOD(SEASCLICK_RES_NAME, write);
static PHP_METHOD(SEASCLICK_RES_NAME, writeEnd);
static PHP_METHOD(SEASCLICK_RES_NAME, execute);

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_construct, 0, 0, 1)
ZEND_ARG_INFO(0, connectParames)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(SeasCilck_destruct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_select, 0, 0, 2)
ZEND_ARG_INFO(0, sql)
ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_insert, 0, 0, 3)
ZEND_ARG_INFO(0, table)
ZEND_ARG_INFO(0, columns)
ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_writeStart, 0, 0, 2)
ZEND_ARG_INFO(0, table)
ZEND_ARG_INFO(0, columns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_write, 0, 0, 1)
ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_writeEnd, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SeasCilck_execute, 0, 0, 2)
ZEND_ARG_INFO(0, sql)
ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

/* {{{ SeasClick_functions[] */
const zend_function_entry SeasClick_functions[] =
{
    //PHP_FE(SeasClick_version,	NULL)
    PHP_FE_END
};
/* }}} */

const zend_function_entry SeasClick_methods[] =
{
    PHP_ME(SEASCLICK_RES_NAME, __construct,   SeasCilck_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(SEASCLICK_RES_NAME, __destruct,    SeasCilck_destruct, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, select,        SeasCilck_select, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, insert,        SeasCilck_insert, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, writeStart,    SeasCilck_writeStart, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, write,         SeasCilck_write, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, writeEnd,      SeasCilck_writeEnd, ZEND_ACC_PUBLIC)
    PHP_ME(SEASCLICK_RES_NAME, execute,       SeasCilck_execute, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

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

#if PHP_VERSION_ID <= 70000
    zend_declare_property_stringl(SeasClick_ce, "host", strlen("host"), "127.0.0.1", sizeof("127.0.0.1") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(SeasClick_ce, "port", strlen("port"), 9000, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_stringl(SeasClick_ce, "database", strlen("database"), "default", sizeof("default") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "user", strlen("user"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "passwd", strlen("passwd"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_bool(SeasClick_ce, "compression", strlen("compression"), false, ZEND_ACC_PROTECTED TSRMLS_CC);
#else
     zend_declare_property_stringl(SeasClick_ce, "host", strlen("host"), "127.0.0.1", sizeof("127.0.0.1") - 1, ZEND_ACC_PROTECTED );
    zend_declare_property_long(SeasClick_ce, "port", strlen("port"), 9000, ZEND_ACC_PROTECTED );
    zend_declare_property_stringl(SeasClick_ce, "database", strlen("database"), "default", sizeof("default") - 1, ZEND_ACC_PROTECTED );
    zend_declare_property_null(SeasClick_ce, "user", strlen("user"), ZEND_ACC_PROTECTED );
    zend_declare_property_null(SeasClick_ce, "passwd", strlen("passwd"), ZEND_ACC_PROTECTED );
    zend_declare_property_bool(SeasClick_ce, "compression", strlen("compression"), false, ZEND_ACC_PROTECTED );
#endif

   

    SeasClick_ce->ce_flags |= ZEND_ACC_FINAL;
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(SeasClick)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(SeasClick)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(SeasClick)
{
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
    PHP_MSHUTDOWN(SeasClick),
    PHP_RINIT(SeasClick),
    PHP_RSHUTDOWN(SeasClick),
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
        sc_zend_update_property_string(SeasClick_ce, this_obj, "host", sizeof("host") - 1, Z_STRVAL_P(value));
    }

    if (php_array_get_value(_ht, "port", value))
    {
        convert_to_long(value);
        sc_zend_update_property_long(SeasClick_ce, this_obj, "port", sizeof("port") - 1, Z_LVAL_P(value));
    }

    if (php_array_get_value(_ht, "compression", value))
    {
        convert_to_boolean(value);
        sc_zend_update_property_long(SeasClick_ce, this_obj, "compression", sizeof("compression") - 1, Z_LVAL_P(value));
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
        sc_zend_update_property_string(SeasClick_ce, this_obj, "database", sizeof("database") - 1, Z_STRVAL_P(value));
        Options = Options.SetDefaultDatabase(Z_STRVAL_P(value));
    }

    if (php_array_get_value(_ht, "user", value))
    {
        convert_to_string(value);
        sc_zend_update_property_string(SeasClick_ce, this_obj, "user", sizeof("user") - 1, Z_STRVAL_P(value));
        Options = Options.SetUser(Z_STRVAL_P(value));
    }

    if (php_array_get_value(_ht, "passwd", value))
    {
        convert_to_string(value);
        sc_zend_update_property_string(SeasClick_ce, this_obj, "passwd", sizeof("passwd") - 1, Z_STRVAL_P(value));
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
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
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

/* {{{ proto array select(string sql, array params)
 */
PHP_METHOD(SEASCLICK_RES_NAME, select)
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
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        if (clientInsertBlack.count(key))
        {
            throw std::runtime_error("The insert operation is now in progress");
        }

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

        array_init(return_value);

        client->Select(sql_s, [return_value](const Block& block)
        {
            zval *return_tmp;
            for (size_t row = 0; row < block.GetRowCount(); ++row)
            {
                SC_MAKE_STD_ZVAL(return_tmp);
                array_init(return_tmp);
                for (size_t column = 0; column < block.GetColumnCount(); ++column)
                {
                    string column_name = block.GetColumnName(column);
                    convertToZval(return_tmp, block[column], row, column_name, 0);
                }
                add_next_index_zval(return_value, return_tmp);
            }
        }
                      );

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
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
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        if (clientInsertBlack.count(key))
        {
            throw std::runtime_error("The insert operation is now in progress");
        }

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
        client->InsertDataEnd();
        sc_zval_ptr_dtor(&return_should);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto array insert(string table, array columns, array values)
 */
PHP_METHOD(SEASCLICK_RES_NAME, writeStart)
{
    char *table = NULL;
    size_t l_table = 0;
    zval *columns;

    string sql;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &table, &l_table, &columns) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(table, l_table)
    Z_PARAM_ARRAY(columns)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif

    try
    {
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        if (clientInsertBlack.count(key))
        {
            throw std::runtime_error("The insert operation is now in progress");
        }

        getInsertSql(&sql, table, columns);
        Block blockQuery;

        client->InsertQuery(sql, [&blockQuery](const Block& block)
        {
            blockQuery = block;
        }
                           );
        
        clientInsertBlack.insert(std::pair<int, Block>(key, blockQuery));
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto array insert(string table, array columns, array values)
 */
PHP_METHOD(SEASCLICK_RES_NAME, write)
{
    zval *values;

#ifndef FAST_ZPP
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &values) == FAILURE)
    {
        return;
    }
#else
#undef IS_UNDEF
#define IS_UNDEF Z_EXPECTED_LONG
    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY(values)
    ZEND_PARSE_PARAMETERS_END();
#undef IS_UNDEF
#define IS_UNDEF 0
#endif

    try
    {
#if PHP_VERSION_ID < 70000
        zval **first_data;
#else
        zval *first_data;
#endif
        HashTable *values_ht = Z_ARRVAL_P(values);
        sc_zend_hash_get_current_data(values_ht, (void**) &first_data);
        if (NULL == first_data)
        {
            throw std::runtime_error("The conut of data inserted is empty");
        }
#if PHP_VERSION_ID < 70000
        size_t columns_count = zend_hash_num_elements(Z_ARRVAL_P(*first_data));
#else
        size_t columns_count = zend_hash_num_elements(Z_ARRVAL_P(first_data));
#endif
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


        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        Block blockQuery = clientInsertBlack.at(key);

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
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto array insert(string table, array columns, array values)
 */
PHP_METHOD(SEASCLICK_RES_NAME, writeEnd)
{
    try
    {
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);
        clientInsertBlack.erase(key);

        client->InsertDataEnd();
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
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
        int key = Z_OBJ_HANDLE(*getThis());
        Client *client = clientMap.at(key);

        if (clientInsertBlack.count(key))
        {
            throw std::runtime_error("The insert operation is now in progress");
        }

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

        client->Execute(sql_s);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
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
        clientInsertBlack.erase(key);

    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception_tsrmls_cc(NULL, e.what(), 0);
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
