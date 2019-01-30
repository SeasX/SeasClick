#if PHP_MAJOR_VERSION < 7
#define IS_TRUE                               1
#define SC_MAKE_STD_ZVAL(p)                   MAKE_STD_ZVAL(p)
#define sc_zval_ptr_dtor                      zval_ptr_dtor
#define sc_zval_add_ref(a)                       zval_add_ref(&a)
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

#define sc_zend_read_property                  zend_read_property

#define SC_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, entry)\
    zval **tmp = NULL; ulong_t idx;\
    for (zend_hash_internal_pointer_reset(ht); \
            (ktype = zend_hash_get_current_key_ex(ht, &k, &klen, &idx, 0, NULL)) != HASH_KEY_NON_EXISTENT; \
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

#define sc_zend_hash_find   zend_hash_str_find
#define sc_zend_hash_index_find   zend_hash_index_find
#define SC_MAKE_STD_ZVAL(p)             zval _stack_zval_##p; p = &(_stack_zval_##p)
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
