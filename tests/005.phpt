--TEST--
SeasClick testNullAble
--SKIPIF--
<?php if (!extension_loaded("SeasClick")) print "skip"; ?>
--FILE--
<?php
$config = [
    "host" => "clickhouse",
    "port" => "9000",
    "compression" => true
];

clientTest($config);

function clientTest($config)
{
    $deleteTable = true;
    $client = new SeasClick($config);
    $client->execute("CREATE DATABASE IF NOT EXISTS test");

    testNullAble($client, $deleteTable);
}

function testNullAble($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.nullable_test (int8null_c Nullable(Int8), stringnull_c Nullable(String), enumnull_c Nullable(Enum8('One8' = 1, 'Two8' = 2)), float32null_c Nullable(Float32), uuidnull_c Nullable(UUID)) ENGINE = Memory");

    $client->insert("test.nullable_test",[
        'int8null_c',
        'stringnull_c',
        'enumnull_c',
        'float32null_c',
        'uuidnull_c'
    ], [
        [8, 'string', 'One8', 32, '31249a1b7b0542709f37c609b48a9bb2'],
        [null, null, null, null, null],
    ]);

    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'int8null_c, stringnull_c, enumnull_c, float32null_c, uuidnull_c',
        'table' => 'test.nullable_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.nullable_test'
        ]);
    }
}

?>
--EXPECT--
array(2) {
  [0]=>
  array(5) {
    ["int8null_c"]=>
    int(8)
    ["stringnull_c"]=>
    string(6) "string"
    ["enumnull_c"]=>
    string(4) "One8"
    ["float32null_c"]=>
    float(32)
    ["uuidnull_c"]=>
    string(32) "31249a1b7b0542709f37c609b48a9bb2"
  }
  [1]=>
  array(5) {
    ["int8null_c"]=>
    NULL
    ["stringnull_c"]=>
    NULL
    ["enumnull_c"]=>
    NULL
    ["float32null_c"]=>
    NULL
    ["uuidnull_c"]=>
    NULL
  }
}
