--TEST--
SeasClick testInt
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

    testUInt($client, $deleteTable);
}

function testUInt($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.int_test (int8_c Int8, int16_c Int16, uint8_c UInt8, uint16_c UInt16) ENGINE = Memory");

    $client->insert("test.int_test",[
        'int8_c','int16_c','uint8_c','uint16_c'
    ], [
        [8, 8, 8, 8],
        [9, 9, 9, 9],
    ]);

    $client->insert("test.int_test",[
        'int8_c','int16_c','uint8_c'
    ], [
        [8, 8, 8],
        [9, 9, 9],
    ]);
    
    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'int8_c, int16_c, uint8_c, uint16_c',
        'table' => 'test.int_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.int_test'
        ]);
    }
}

?>
--EXPECT--
array(4) {
  [0]=>
  array(4) {
    ["int8_c"]=>
    int(8)
    ["int16_c"]=>
    int(8)
    ["uint8_c"]=>
    int(8)
    ["uint16_c"]=>
    int(8)
  }
  [1]=>
  array(4) {
    ["int8_c"]=>
    int(9)
    ["int16_c"]=>
    int(9)
    ["uint8_c"]=>
    int(9)
    ["uint16_c"]=>
    int(9)
  }
  [2]=>
  array(4) {
    ["int8_c"]=>
    int(8)
    ["int16_c"]=>
    int(8)
    ["uint8_c"]=>
    int(8)
    ["uint16_c"]=>
    int(0)
  }
  [3]=>
  array(4) {
    ["int8_c"]=>
    int(9)
    ["int16_c"]=>
    int(9)
    ["uint8_c"]=>
    int(9)
    ["uint16_c"]=>
    int(0)
  }
}
