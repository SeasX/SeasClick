SeasClick
=====
[![Build Status](https://travis-ci.org/SeasX/SeasClick.svg?branch=master)](https://travis-ci.org/SeasX/SeasClick)

PHP client for [Yandex ClickHouse](https://clickhouse.yandex/)，Based on [ClickHouse C++ client](https://github.com/aiwhj/clickhouse-cpp)

## ClickHouse
* [What is ClickHouse](https://clickhouse.yandex/docs/en/)
* [ClickHouse Performance](https://clickhouse.yandex/docs/en/introduction/performance/)
* [Performance comparison with MySQL](https://clickhouse.yandex/benchmark.html#[%22100000000%22,[%22ClickHouse%22,%22MySQL%22],[%220%22,%221%22]])

## Supported data types

* Array(T)
    **Multidimensional arrays are not supported at this time**
* Date
* DateTime
* Enum8, Enum16
* FixedString(N)
* Float32, Float64
* Nullable(T)
* String
* UInt8, UInt16, UInt32, UInt64, Int8, Int16, Int32, Int64

## Supported PHP version
PHP 5.5+

## Performance
![image](https://github.com/SeasX/SeasClick/raw/master/tests/bench_mark/bench_mark.png)

This performance test [demo](https://github.com/SeasX/SeasClick/blob/master/tests/bench_mark/bench_mark.php) is compared to [phpclickhouse](https://github.com/smi2/phpClickHouse)

## Install
```ssh
git clone https://github.com/SeasX/SeasClick.git
git submodule init
git submodule update
cd SeasClick
phpize
./configure
make && make install
```

## Example

```php
<?php
$config = [
    "host" => "clickhouse",
    "port" => 9000,
    "compression" => true
];

clientTest($config);

function clientTest($config)
{
    $deleteTable = true;
    $client = new SeasClick($config);

    $client->execute("CREATE DATABASE IF NOT EXISTS test");

    testArray($client, $deleteTable);
}

function testArray($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.array_test (string_c String, array_c Array(Int8), arraynull_c Array(Nullable(String))) ENGINE = Memory");

    $client->insert("test.array_test", [
        'string_c', 'array_c', 'arraynull_c'
    ], [
        ['string_c1', [1, 2, 3], ['string']],
        ['string_c2', [4, 5, 6], [null]]
    ]);

    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'string_c, array_c, arraynull_c',
        'table' => 'test.array_test'
    ]);
    var_dump($result);

    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.array_test'
        ]);
    }
}
```
#### [More examples](https://github.com/SeasX/SeasClick/blob/master/tests/test.php)

## Support
SeasX Group
