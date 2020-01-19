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

#include "ext/swoole/config.h"
#include "ext/swoole/php_swoole_cxx.h"

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
using namespace swoole;

static zend_object_handlers swoole_seasclick_coro_handlers;

typedef struct _php_seasclick_object
{
    swSocket *socket;
    php_coro_context context;
    Client *client;
    enum query_type request_type;
    zend_object std;
} sc_object_t;

zend_class_entry *SeasClick_ce;

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

static int swoole_seasclick_coro_onRead(swReactor *reactor, swEvent *event);
static int swoole_seasclick_coro_onWrite(swReactor *reactor, swEvent *event);
static int swoole_seasclick_coro_onError(swReactor *reactor, swEvent *event);


static sw_inline sc_object_t* php_seasclick_coro_fetch_object(zval *zobject);
static zend_object *php_seasclick_coro_create_object(zend_class_entry *ce);


ZEND_BEGIN_ARG_INFO_EX(SeasCilck_construct, 0, 0, 1)
ZEND_ARG_INFO(0, connectParames)
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
    memcpy(&swoole_seasclick_coro_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    swoole_seasclick_coro_handlers.offset = XtOffsetOf(sc_object_t, std);

	SeasClick_ce->create_object = php_seasclick_coro_create_object;
	SeasClick_ce->serialize = zend_class_serialize_deny;
	SeasClick_ce->unserialize = zend_class_unserialize_deny;

    zend_declare_property_stringl(SeasClick_ce, "host", strlen("host"), "127.0.0.1", sizeof("127.0.0.1") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(SeasClick_ce, "port", strlen("port"), 9000, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_stringl(SeasClick_ce, "database", strlen("database"), "default", sizeof("default") - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "user", strlen("user"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(SeasClick_ce, "passwd", strlen("passwd"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_bool(SeasClick_ce, "compression", strlen("compression"), false, ZEND_ACC_PROTECTED TSRMLS_CC);

    SeasClick_ce->ce_flags = ZEND_ACC_IMPLICIT_PUBLIC;
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

static sw_inline sc_object_t* php_seasclick_coro_fetch_object(zval *zobject)
{
    return (sc_object_t *) ((char *) Z_OBJ_P(zobject) - swoole_seasclick_coro_handlers.offset);
}

static zend_object *php_seasclick_coro_create_object(zend_class_entry *ce)
{
    sc_object_t *seasclick_coro = (sc_object_t *) ecalloc(1, sizeof(sc_object_t) + zend_object_properties_size(ce));
    zend_object_std_init(&seasclick_coro->std, ce);
    object_properties_init(&seasclick_coro->std, ce);
    seasclick_coro->std.handlers = &swoole_seasclick_coro_handlers;

    return &seasclick_coro->std;
}

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
                            .SetNonBlocking(true)
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

        php_swoole_check_reactor();

        if (!swReactor_isset_handler(sw_reactor(), PHP_SWOOLE_FD_POSTGRESQL))
        {
            swReactor_set_handler(sw_reactor(), PHP_SWOOLE_FD_POSTGRESQL | SW_EVENT_READ, swoole_seasclick_coro_onRead);
            swReactor_set_handler(sw_reactor(), PHP_SWOOLE_FD_POSTGRESQL | SW_EVENT_WRITE, swoole_seasclick_coro_onWrite);
            swReactor_set_handler(sw_reactor(), PHP_SWOOLE_FD_POSTGRESQL | SW_EVENT_ERROR, swoole_seasclick_coro_onError);
        }

        sc_object_t *seasclick_coro = php_seasclick_coro_fetch_object(ZEND_THIS);
        seasclick_coro->client = client;
        seasclick_coro->request_type = query_type::Connect;

        swSocket *socket = swSocket_new(client->GetFd(), (enum swFd_type) PHP_SWOOLE_FD_POSTGRESQL);

        if (sw_reactor()->add(sw_reactor(), socket, SW_EVENT_WRITE) < 0)
        {
            php_swoole_fatal_error(E_WARNING, "swoole_event_add failed");
            RETURN_FALSE;
        }

        socket->object = seasclick_coro;

        seasclick_coro->socket = socket;

        php_coro_context *context = &seasclick_coro->context;
        context->coro_params = *ZEND_THIS;

        PHPCoroutine::yield_m(return_value, context);
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
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

        sc_object_t *seasclick_coro = php_seasclick_coro_fetch_object(ZEND_THIS);
        Client *client = seasclick_coro->client;

        client->Select(sql_s);

        seasclick_coro->request_type = query_type::SelectQuery;

        php_coro_context *context = &seasclick_coro->context;
        context->coro_params = *ZEND_THIS;

        swoole_event_add(seasclick_coro->socket, SW_EVENT_READ);
        PHPCoroutine::yield_m(return_value, context);
        zval_ptr_dtor(return_value);
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

        sc_object_t *seasclick_coro = php_seasclick_coro_fetch_object(ZEND_THIS);
        Client *client = seasclick_coro->client;

        client->InsertQuery(sql);

        seasclick_coro->request_type = query_type::InsertQuery;

        php_coro_context *context = &seasclick_coro->context;
        context->coro_params = *return_should;

        swoole_event_add(seasclick_coro->socket, SW_EVENT_READ);
        PHPCoroutine::yield_m(return_value, context);

        sc_zval_ptr_dtor(&return_should);

        seasclick_coro->request_type = query_type::InsertData;
        swoole_event_add(seasclick_coro->socket, SW_EVENT_READ);
        PHPCoroutine::yield_m(return_value, context);
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

        sc_object_t *seasclick_coro = php_seasclick_coro_fetch_object(ZEND_THIS);
        Client *client = seasclick_coro->client;
        client->Execute(sql_s);

        seasclick_coro->request_type = query_type::ExecuteQuery;

        php_coro_context *context = &seasclick_coro->context;
        context->coro_params = *ZEND_THIS;

        swoole_event_add(seasclick_coro->socket, SW_EVENT_READ);
        PHPCoroutine::yield_m(return_value, context);
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
}
/* }}} */

/* {{{ proto array __destruct()
 */
PHP_METHOD(SEASCLICK_RES_NAME, __destruct)
{
    try
    {
        sc_object_t *seasclick_coro = php_seasclick_coro_fetch_object(ZEND_THIS);
        Client *client = seasclick_coro->client;
        delete client;
    }
    catch (const std::exception& e)
    {
        sc_zend_throw_exception(NULL, e.what(), 0 TSRMLS_CC);
    }
    RETURN_TRUE;
}
/* }}} */

static int swoole_seasclick_coro_onRead(swReactor *reactor, swEvent *event)
{
    sc_object_t *seasclick_coro = (sc_object_t *)(event->socket->object);
    swoole_event_del(event->socket);

    Client *client = seasclick_coro->client;
    zval *_return_value = NULL;

    if (seasclick_coro->request_type == query_type::SelectQuery) {
        zval return_value;
        _return_value = &return_value;
        array_init(_return_value);
        client->ReadData([_return_value](const Block& block)
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
                add_next_index_zval(_return_value, return_tmp);
            }
        });
    } else if (seasclick_coro->request_type == query_type::ExecuteQuery) {
        client->ReadData();
    } else if (seasclick_coro->request_type == query_type::InsertQuery) {

        Block blockQuery;
        client->ReadData([&blockQuery](const Block& block)
        {
            blockQuery = block;
        });

        Block blockInsert;
        size_t index = 0;
        php_coro_context *context = &seasclick_coro->context;
        zval *return_should = &context->coro_params;

        zval *pzval;
        char *str_key;
        uint32_t str_keylen;
        int keytype;
        SC_HASHTABLE_FOREACH_START2(Z_ARRVAL_P(return_should), str_key, str_keylen, keytype, pzval)
        {
            zvalToBlock(blockInsert, blockQuery, index, pzval);
            index++;
        }
        SC_HASHTABLE_FOREACH_END();

        client->InsertData(blockInsert);
    } else if (seasclick_coro->request_type == query_type::InsertData) {
        client->ReadData();
    }

    zval *retval = NULL;
    int ret = PHPCoroutine::resume_m(&seasclick_coro->context, _return_value, retval);
    if (ret == SW_CORO_ERR_END && retval)
    {
        zval_ptr_dtor(retval);
    }
    return SW_OK;
}

static int swoole_seasclick_coro_onWrite(swReactor *reactor, swEvent *event)
{
    sc_object_t *seasclick_coro = (sc_object_t *)(event->socket->object);
    swoole_event_del(event->socket);

    zval _result;
    zval *result = &_result;
    zval *retval = NULL;

    ZVAL_FALSE(result);
    
    int ret = PHPCoroutine::resume_m(&seasclick_coro->context, result, retval);
    zval_ptr_dtor(result);

    if (ret == SW_CORO_ERR_END && retval)
    {
        zval_ptr_dtor(retval);
    }
    return SW_OK;
}

static int swoole_seasclick_coro_onError(swReactor *reactor, swEvent *event)
{
    sc_object_t *seasclick_coro = (sc_object_t *)(event->socket->object);
    swoole_event_del(event->socket);

    zval _result;
    zval *result = &_result;
    zval *retval = NULL;

    ZVAL_FALSE(result);

    int ret = PHPCoroutine::resume_m(&seasclick_coro->context, result, retval);
    zval_ptr_dtor(result);

    if (ret == SW_CORO_ERR_END && retval)
    {
        zval_ptr_dtor(retval);
    }

    return SW_OK;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
