ClickHouse C++ client [![Build Status](https://travis-ci.org/artpaul/clickhouse-cpp.svg?branch=master)](https://travis-ci.org/artpaul/clickhouse-cpp)
=====

C++ client for [Yandex ClickHouse](https://clickhouse.yandex/)

## This repositorie change
* [Add InsertQuery and InsertData methods](https://github.com/aiwhj/clickhouse-cpp/commit/bab28bcb5a509d80b8e2e0c7e89512446283dde5)
* [add tupleSize Function](https://github.com/aiwhj/clickhouse-cpp/commit/7c75a6a711432469a0e371b83ebc0586f4e5ecad)
* [add GetTupleType Function](https://github.com/aiwhj/clickhouse-cpp/commit/7f24a86f65b1a3316a49babab9a8589670a7d763)
* [support multidimensional arrays](https://github.com/aiwhj/clickhouse-cpp/commit/fff57d94b83240faef5ba61ccf3e1ec129230a05)


## Supported data types

* Array(T)
* Date
* DateTime
* Enum8, Enum16
* FixedString(N)
* Float32, Float64
* Nullable(T)
* String
* Tuple
* UInt8, UInt16, UInt32, UInt64, Int8, Int16, Int32, Int64

## Building

```sh
$ mkdir build .
$ cd build
$ cmake ..
$ make
```

## Example

```cpp
#include <clickhouse/client.h>

using namespace clickhouse;

/// Initialize client connection.
Client client(ClientOptions().SetHost("localhost"));

/// Create a table.
client.Execute("CREATE TABLE IF NOT EXISTS test.numbers (id UInt64, name String) ENGINE = Memory");

/// Insert some values.
{
    Block block;

    auto id = std::make_shared<ColumnUInt64>();
    id->Append(1);
    id->Append(7);

    auto name = std::make_shared<ColumnString>();
    name->Append("one");
    name->Append("seven");

    block.AppendColumn("id"  , id);
    block.AppendColumn("name", name);

    client.Insert("test.numbers", block);
}

/// Select values inserted in the previous step.
client.Select("SELECT id, name FROM test.numbers", [] (const Block& block)
    {
        for (size_t i = 0; i < block.GetRowCount(); ++i) {
            std::cout << block[0]->As<ColumnUInt64>()->At(i) << " "
                      << block[1]->As<ColumnString>()->At(i) << "\n";
        }
    }
);

/// Delete table.
client.Execute("DROP TABLE test.numbers");
```
