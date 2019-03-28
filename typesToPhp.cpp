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
#include "php7_wrapper.h"
};

#include "php_SeasClick.h"

#include "lib/clickhouse-cpp/clickhouse/client.h"
#include "lib/clickhouse-cpp/clickhouse/error_codes.h"
#include "lib/clickhouse-cpp/clickhouse/types/type_parser.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "typesToPhp.hpp"

using namespace clickhouse;
using namespace std;

ColumnRef createColumn(TypeRef type)
{
    switch (type->GetCode())
    {
    case Type::Code::UInt64:
    {
        return std::make_shared<ColumnUInt64>();
    }
    case Type::Code::UInt8:
    {
        return std::make_shared<ColumnUInt8>();
    }
    case Type::Code::UInt16:
    {
        return std::make_shared<ColumnUInt16>();
    }
    case Type::Code::UInt32:
    {
        return std::make_shared<ColumnUInt32>();
    }

    case Type::Code::Int8:
    {
        return std::make_shared<ColumnInt8>();
    }
    case Type::Code::Int16:
    {
        return std::make_shared<ColumnInt16>();
    }
    case Type::Code::Int32:
    {
        return std::make_shared<ColumnInt32>();
    }
    case Type::Code::Int64:
    {
        return std::make_shared<ColumnInt64>();
    }

    case Type::Code::UUID:
    {
        return std::make_shared<ColumnUUID>();
    }

    case Type::Code::Float32:
    {
        return std::make_shared<ColumnFloat32>();
    }
    case Type::Code::Float64:
    {
        return std::make_shared<ColumnFloat64>();
    }

    case Type::Code::String:
    {
        return std::make_shared<ColumnString>();
    }
    case Type::Code::FixedString:
    {
        string typeName = type->GetName();
        typeName.erase(typeName.find("FixedString("), 12);
        typeName.erase(typeName.find(")"), 1);
        return std::make_shared<ColumnFixedString>(std::stoi(typeName));
    }

    case Type::Code::DateTime:
    {
        return std::make_shared<ColumnDateTime>();
    }
    case Type::Code::Date:
    {
        return std::make_shared<ColumnDate>();
    }

    case Type::Code::Array:
    {
        return std::make_shared<ColumnArray>(createColumn(type->GetItemType()));
    }

    case Type::Code::Enum8:
    {
        std::vector<Type::EnumItem> enum_items;

        auto enumType = EnumType(type);

        for (auto ei = enumType.BeginValueToName(); ; )
        {
            enum_items.push_back(
                Type::EnumItem {ei->second, (int8_t)ei->first});
            if (++ei == enumType.EndValueToName())
            {
                break;
            }
        }

        return std::make_shared<ColumnEnum8>(Type::CreateEnum8(enum_items));
    }
    case Type::Code::Enum16:
    {
        std::vector<Type::EnumItem> enum_items;

        auto enumType = EnumType(type);

        for (auto ei = enumType.BeginValueToName(); ; )
        {
            enum_items.push_back(
                Type::EnumItem {ei->second, (int16_t)ei->first});
            if (++ei == enumType.EndValueToName())
            {
                break;
            }
        }

        return std::make_shared<ColumnEnum16>(Type::CreateEnum16(enum_items));
    }

    case Type::Code::Nullable:
    {
        return std::make_shared<ColumnNullable>(createColumn(type->GetNestedType()), std::make_shared<ColumnUInt8>());
    }

    case Type::Code::Tuple:
    {
        throw std::runtime_error("can't support Tuple");
    }

    case Type::Code::Void:
    {
        throw std::runtime_error("can't support Void");
    }
    }

    throw std::runtime_error("createColumn runtime error.");
}

ColumnRef insertColumn(TypeRef type, zval *value_zval)
{
    zval *array_value;
    char *str_key;
    uint32_t str_keylen;
    int keytype;

    HashTable *values_ht = Z_ARRVAL_P(value_zval);

    switch (type->GetCode())
    {
    case Type::Code::UInt64:
    {
        auto value = std::make_shared<ColumnUInt64>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::UInt8:
    {
        auto value = std::make_shared<ColumnUInt8>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::UInt16:
    {
        auto value = std::make_shared<ColumnUInt16>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::UInt32:
    {
        auto value = std::make_shared<ColumnUInt32>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::Int8:
    {
        auto value = std::make_shared<ColumnInt8>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Int16:
    {
        auto value = std::make_shared<ColumnInt16>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Int32:
    {
        auto value = std::make_shared<ColumnInt32>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Int64:
    {
        auto value = std::make_shared<ColumnInt64>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::UUID:
    {
        auto value = std::make_shared<ColumnUUID>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            if (Z_TYPE_P(array_value) == IS_NULL)
            {
                value->Append(UInt128(0, 0));
            }
            else
            {
                convert_to_string(array_value);
                string value_string = (string)Z_STRVAL_P(array_value);

                value_string.erase(std::remove(value_string.begin(), value_string.end(), '-'), value_string.end());
                if (value_string.length() != 32)
                {
                    throw std::runtime_error("UUID format error");
                }

                string first = value_string.substr(0, 16);
                string second = value_string.substr(16, 16);
                uint64_t i_first = std::stoull(first, nullptr, 16);
                uint64_t i_second = std::stoull(second, nullptr, 16);
                value->Append(UInt128(i_first, i_second));
            }
        }
        SC_HASHTABLE_FOREACH_END();
        return value;
        break;
    }

    case Type::Code::Float32:
    {
        auto value = std::make_shared<ColumnFloat32>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_double(array_value);
            value->Append(Z_DVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Float64:
    {
        auto value = std::make_shared<ColumnFloat64>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_double(array_value);
            value->Append(Z_DVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::String:
    {
        auto value = std::make_shared<ColumnString>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_string(array_value);
            value->Append((string)Z_STRVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
    }
    case Type::Code::FixedString:
    {
        string typeName = type->GetName();
        typeName.erase(typeName.find("FixedString("), 12);
        typeName.erase(typeName.find(")"), 1);
        auto value = std::make_shared<ColumnFixedString>(std::stoi(typeName));

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_string(array_value);
            value->Append((string)Z_STRVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
    }

    case Type::Code::DateTime:
    {
        auto value = std::make_shared<ColumnDateTime>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Date:
    {
        auto value = std::make_shared<ColumnDate>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            convert_to_long(array_value);
            value->Append(Z_LVAL_P(array_value));
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::Array:
    {
        if (type->GetItemType()->GetCode() == Type::Array)
        {
            throw std::runtime_error("can't support Multidimensional Arrays");
        }

        auto value = std::make_shared<ColumnArray>(createColumn(type->GetItemType()));
        auto child = createColumn(type->GetItemType());

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            if (Z_TYPE_P(array_value) != IS_ARRAY)
            {
                throw std::runtime_error("The inserted data is not an array type");
            }

            child->Append(insertColumn(type->GetItemType(), array_value));

            value->AppendAsColumn(child);
            child->Clear();
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::Enum8:
    {
        std::vector<Type::EnumItem> enum_items;

        auto enumType = EnumType(type);

        for (auto ei = enumType.BeginValueToName(); ; )
        {
            enum_items.push_back(
                Type::EnumItem {ei->second, (int8_t)ei->first});
            if (++ei == enumType.EndValueToName())
            {
                break;
            }
        }

        auto value = std::make_shared<ColumnEnum8>(Type::CreateEnum8(enum_items));

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            if (Z_TYPE_P(array_value) == IS_LONG)
            {
                convert_to_long(array_value);
                value->Append(Z_LVAL_P(array_value));
            }
            else
            {
                convert_to_string(array_value);
                value->Append((string)Z_STRVAL_P(array_value));
            }
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }
    case Type::Code::Enum16:
    {
        std::vector<Type::EnumItem> enum_items;

        auto enumType = EnumType(type);

        for (auto ei = enumType.BeginValueToName(); ; )
        {
            enum_items.push_back(
                Type::EnumItem {ei->second, (int16_t)ei->first});
            if (++ei == enumType.EndValueToName())
            {
                break;
            }
        }

        auto value = std::make_shared<ColumnEnum16>(Type::CreateEnum16(enum_items));

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            if (Z_TYPE_P(array_value) == IS_LONG)
            {
                convert_to_long(array_value);
                value->Append(Z_LVAL_P(array_value));
            }
            else
            {
                convert_to_string(array_value);
                value->Append((string)Z_STRVAL_P(array_value));
            }
        }
        SC_HASHTABLE_FOREACH_END();

        return value;
        break;
    }

    case Type::Code::Nullable:
    {
        auto nulls = std::make_shared<ColumnUInt8>();

        SC_HASHTABLE_FOREACH_START2(values_ht, str_key, str_keylen, keytype, array_value)
        {
            if (Z_TYPE_P(array_value) == IS_NULL)
            {
                nulls->Append(1);
            }
            else
            {
                nulls->Append(0);
            }
        }
        SC_HASHTABLE_FOREACH_END();

        ColumnRef child = insertColumn(type->GetNestedType(), value_zval);

        return std::make_shared<ColumnNullable>(child, nulls);
        break;
    }

    case Type::Code::Tuple:
    {
        throw std::runtime_error("can't support Tuple");
    }

    case Type::Code::Void:
    {
        throw std::runtime_error("can't support Void");
    }
    }

    throw std::runtime_error("insertColumn runtime error.");
}

void convertToZval(zval *arr, const ColumnRef& columnRef, int row, string column_name, int8_t is_array)
{
    switch (columnRef->Type()->GetCode())
    {
    case Type::Code::UInt64:
    {
        auto col = (*columnRef->As<ColumnUInt64>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::UInt8:
    {
        auto col = (*columnRef->As<ColumnUInt8>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::UInt16:
    {
        auto col = (*columnRef->As<ColumnUInt16>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::UInt32:
    {
        auto col = (*columnRef->As<ColumnUInt32>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }

    case Type::Code::Int8:
    {
        auto col = (*columnRef->As<ColumnInt8>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::Int16:
    {
        auto col = (*columnRef->As<ColumnInt16>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::Int32:
    {
        auto col = (*columnRef->As<ColumnInt32>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }
    case Type::Code::Int64:
    {
        auto col = (*columnRef->As<ColumnInt64>())[row];
        if (is_array)
        {
            add_next_index_long(arr, (long)col);
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col);
        }
        break;
    }

    case Type::Code::UUID:
    {
        stringstream first;
        stringstream second;
        auto col = (*columnRef->As<ColumnUUID>())[row];
        first<<std::setw(16)<<std::setfill('0')<<hex<<col.first;
        second<<std::setw(16)<<std::setfill('0')<<hex<<col.second;
        if (is_array)
        {
            sc_add_next_index_stringl(arr, (char*)(first.str() + second.str()).c_str(), (first.str() + second.str()).length(), 1);
        }
        else
        {
            sc_add_assoc_stringl_ex(arr, column_name.c_str(), column_name.length(), (char*)(first.str() + second.str()).c_str(), (first.str() + second.str()).length(), 1);
        }
        break;
    }

    case Type::Code::Float32:
    {
        auto col = (*columnRef->As<ColumnFloat32>())[row];
        stringstream stream;
        stream<<col;
        double d;
        stream>>d;
        if (is_array)
        {
            add_next_index_double(arr, d);
        }
        else
        {
            sc_add_assoc_double_ex(arr, column_name.c_str(), column_name.length(), d);
        }
        break;
    }
    case Type::Code::Float64:
    {
        auto col = (*columnRef->As<ColumnFloat64>())[row];
        if (is_array)
        {
            add_next_index_double(arr, (double)col);
        }
        else
        {
            sc_add_assoc_double_ex(arr, column_name.c_str(), column_name.length(), (double)col);
        }
        break;
    }

    case Type::Code::String:
    {
        auto col = (*columnRef->As<ColumnString>())[row];
        if (is_array)
        {
            sc_add_next_index_stringl(arr, (char*)col.c_str(), col.length(), 1);
        }
        else
        {
            sc_add_assoc_stringl_ex(arr, column_name.c_str(), column_name.length(), (char*)col.c_str(), col.length(), 1);
        }
        break;
    }
    case Type::Code::FixedString:
    {
        auto col = (*columnRef->As<ColumnFixedString>())[row];
        if (is_array)
        {
            sc_add_next_index_stringl(arr, (char*)col.c_str(), col.length(), 1);
        }
        else
        {
            sc_add_assoc_stringl_ex(arr, column_name.c_str(), column_name.length(), (char*)col.c_str(), strlen((char*)col.c_str()), 1);
        }
        break;
    }

    case Type::Code::DateTime:
    {
        auto col = columnRef->As<ColumnDateTime>();
        if (is_array)
        {
            add_next_index_long(arr, (long)col->As<ColumnDateTime>()->At(row));
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col->As<ColumnDateTime>()->At(row));
        }
        break;
    }
    case Type::Code::Date:
    {
        auto col = columnRef->As<ColumnDate>();
        if (is_array)
        {
            add_next_index_long(arr, (long)col->As<ColumnDate>()->At(row));
        }
        else
        {
            sc_add_assoc_long_ex(arr, column_name.c_str(), column_name.length(), (zend_ulong)col->As<ColumnDate>()->At(row));
        }
        break;
    }

    case Type::Code::Array:
    {
        auto array = columnRef->As<ColumnArray>();
        auto col = array->GetAsColumn(row);
        zval *return_tmp;
        SC_MAKE_STD_ZVAL(return_tmp);
        array_init(return_tmp);
        for (size_t i = 0; i < col->Size(); ++i)
        {
            convertToZval(return_tmp, col, i, "array", 1);
        }
        if (is_array)
        {
            add_next_index_zval(arr, return_tmp);
        }
        else
        {
            sc_add_assoc_zval_ex(arr, column_name.c_str(), column_name.length(), return_tmp);
        }
        break;
    }

    case Type::Code::Enum8:
    {
        auto array = columnRef->As<ColumnEnum8>();
        if (is_array)
        {
            sc_add_next_index_stringl(arr, (char*)array->NameAt(row).c_str(), array->NameAt(row).length(), 1);
        }
        else
        {
            sc_add_assoc_stringl_ex(arr, column_name.c_str(), column_name.length(), (char*)array->NameAt(row).c_str(), array->NameAt(row).length(), 1);
        }
        break;
    }
    case Type::Code::Enum16:
    {
        auto array = columnRef->As<ColumnEnum16>();
        if (is_array)
        {
            sc_add_next_index_stringl(arr, (char*)array->NameAt(row).c_str(), array->NameAt(row).length(), 1);
        }
        else
        {
            sc_add_assoc_stringl_ex(arr, column_name.c_str(), column_name.length(), (char*)array->NameAt(row).c_str(), array->NameAt(row).length(), 1);
        }
        break;
    }

    case Type::Code::Nullable:
    {
        auto nullable = columnRef->As<ColumnNullable>();
        if (nullable->IsNull(row))
        {
            if (is_array)
            {
                add_next_index_null(arr);
            }
            else
            {
                sc_add_assoc_null_ex(arr, column_name.c_str(), column_name.length());
            }
        }
        else
        {
            convertToZval(arr, nullable->Nested(), row, column_name, 0);
        }
        break;
    }

    case Type::Code::Tuple:
    {
        throw std::runtime_error("can't support Tuple");
    }

    case Type::Code::Void:
    {
        throw std::runtime_error("can't support Void");
    }
    }
}

void zvalToBlock(Block& blockDes, Block& blockSrc, zend_ulong num_key, zval *value_zval)
{
    ColumnRef column = insertColumn(blockSrc[num_key]->Type(), value_zval);

    blockDes.AppendColumn(blockSrc.GetColumnName(num_key), column);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
