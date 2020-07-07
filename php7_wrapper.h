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
// PHP7.4 + 
#if !defined(ZEND_ACC_IMPLICIT_PUBLIC)
# define ZEND_ACC_IMPLICIT_PUBLIC ZEND_ACC_PUBLIC
#endif

// PHP7+
#if PHP_MAJOR_VERSION < 7

typedef long zend_long;

#if PHP_VERSION_ID < 50500
#define sc_zend_throw_exception(a, b, c) zend_throw_exception(a, (char *)b, c)
#else
#define sc_zend_throw_exception zend_throw_exception
#endif

#define IS_TRUE                               1
#define SC_MAKE_STD_ZVAL(p)                   MAKE_STD_ZVAL(p)
#define SC_RETURN_STRINGL(k, l) RETURN_STRINGL(k, l, 1)
#define sc_zval_ptr_dtor                      zval_ptr_dtor
#define sc_zval_add_ref(a)                       zval_add_ref(&a)

static inline int sc_zend_hash_index_update(HashTable *ht, ulong h, void *pData)
{
    return zend_hash_index_update(ht, h, pData, sizeof(zval *), NULL);
}

static inline int sc_add_assoc_long_ex(zval *arg, const char *key, size_t key_len, long value)
{
    return add_assoc_long_ex(arg, key, key_len + 1, value);
}

static inline int sc_add_assoc_double_ex(zval *arg, const char *key, size_t key_len, double value)
{
    return add_assoc_double_ex(arg, key, key_len + 1, value);
}

static inline int sc_add_assoc_zval_ex(zval *arg, const char *key, size_t key_len, zval* value)
{
    return add_assoc_zval_ex(arg, key, key_len + 1, value);
}

static inline int sc_add_assoc_stringl_ex(zval *arg, const char *key, size_t key_len, char *str, size_t length, int __duplicate)
{
    return add_assoc_stringl_ex(arg, key, key_len + 1, str, length, __duplicate);
}

static inline int sc_add_assoc_null_ex(zval *arg, const char *key, size_t key_len)
{
    return add_assoc_null_ex(arg, key, key_len + 1);
}

static inline zval *sc_zend_hash_find(HashTable *ht, char *k, int len)
{
    zval **tmp = NULL;
    if (zend_hash_find(ht, k, len + 1, (void **) &tmp) == SUCCESS)
    {
        return *tmp;
    }
    else
    {
        return NULL;
    }
}

static inline zval *sc_zend_hash_index_find(HashTable *ht, ulong h)
{
    zval **tmp = NULL;
    if (zend_hash_index_find(ht, h, (void **) &tmp) == SUCCESS)
    {
        return *tmp;
    }
    else
    {
        return NULL;
    }
}

#define sc_zend_read_property(a, b, c, d, e)                  zend_read_property(a, b, c, d, e TSRMLS_CC)

#define SC_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, entry)\
    zval **tmp = NULL; ulong_t idx;\
    for (zend_hash_internal_pointer_reset(ht); \
            (ktype = zend_hash_get_current_key_ex(ht, &k, &klen, &idx, 0, NULL)) != HASH_KEY_NON_EXISTANT; \
            zend_hash_move_forward(ht)\
        ) { \
    if (zend_hash_get_current_data(ht, (void**)&tmp) == FAILURE) {\
        continue;\
    }\
    entry = *tmp;\
    klen --;

#define SC_HASHTABLE_FOREACH_END() }

#define sc_add_next_index_stringl             add_next_index_stringl

#else
// PHP5
#define sc_zend_throw_exception zend_throw_exception

#define sc_zend_hash_index_update   zend_hash_index_update
#define sc_zend_hash_find   zend_hash_str_find
#define sc_zend_hash_index_find   zend_hash_index_find
#define SC_MAKE_STD_ZVAL(p)             zval _stack_zval_##p; p = &(_stack_zval_##p)
#define SC_RETURN_STRINGL(k, l) RETURN_STRINGL(k, l)
#define sc_zval_ptr_dtor(p)  zval_ptr_dtor(*p)
#define sc_zval_add_ref(p)   Z_TRY_ADDREF_P(p)
#define sc_add_assoc_long_ex                  add_assoc_long_ex
#define sc_add_assoc_double_ex                add_assoc_double_ex
#define sc_add_assoc_zval_ex                  add_assoc_zval_ex
#define sc_add_assoc_stringl_ex(a, b, c, d, e, f)               add_assoc_stringl_ex(a, b, c, d, e)
#define sc_add_assoc_null_ex(a, b, c)               add_assoc_null_ex(a, b, c)
static inline zval* sc_zend_read_property(zend_class_entry *class_ptr, zval *obj, const char *s, int len, int silent)
{
    zval rv;
    return zend_read_property(class_ptr, obj, s, len, silent, &rv);
}

#define SC_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, _val) zend_string *_foreach_key;\
    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _foreach_key, _val);\
    if (!_foreach_key) {k = NULL; klen = 0; ktype = 0;}\
    else {k = _foreach_key->val, klen=_foreach_key->len; ktype = 1;} {

#define SC_HASHTABLE_FOREACH_END()                 } ZEND_HASH_FOREACH_END();

#define sc_add_next_index_stringl(arr, str, len, dup)    add_next_index_stringl(arr, str, len)

#endif

#define php_array_get_value(ht, str, v) ((v = sc_zend_hash_find(ht, (char *)str, sizeof(str)-1)) && !ZVAL_IS_NULL(v))

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
