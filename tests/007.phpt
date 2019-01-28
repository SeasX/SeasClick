--TEST--
SeasClick testArray
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

    testFloat($client, $deleteTable);
}

function testFloat($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.float_test (float32_c Float32, float64_c Float64) ENGINE = Memory");

    $client->insert("test.float_test",[
        'float32_c', 'float64_c'
    ], [
        [32.32, 64.64],
        [32.31, 64.68]
    ]);

    $client->insert("test.float_test",[
        'float32_c'
    ], [
        [32.32],
        [32.31]
    ]);
    
    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'float32_c, float64_c',
        'table' => 'test.float_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.float_test'
        ]);
    }
}

?>
--EXPECT--
array(4) {
  [0]=>
  array(2) {
    ["float32_c"]=>
    float(32.319999694824)
    ["float64_c"]=>
    float(64.64)
  }
  [1]=>
  array(2) {
    ["float32_c"]=>
    float(32.310001373291)
    ["float64_c"]=>
    float(64.68)
  }
  [2]=>
  array(2) {
    ["float32_c"]=>
    float(32.319999694824)
    ["float64_c"]=>
    float(0)
  }
  [3]=>
  array(2) {
    ["float32_c"]=>
    float(32.310001373291)
    ["float64_c"]=>
    float(0)
  }
}
