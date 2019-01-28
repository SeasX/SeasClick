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

    testUUID($client, $deleteTable);
}

function testUUID($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.uuid_test (uuid_c UUID, uuid2_c UUID) ENGINE = Memory");

    $client->insert("test.uuid_test",[
        'uuid_c', 'uuid2_c'
    ], [
        ['31249a1b-7b05-4270-9f37-c609b48a9bb2', '31249a1b7b0542709f37c609b48a9bb2'],
        ['31249a1b-7b05-4270-9f37-c609b48a9bb2', null],
    ]);

    $client->insert("test.uuid_test",[
        'uuid_c'
    ], [
        ['00000000-0000-0000-9f37-c609b48a9bb2'],
        ['31249a1b-7b05-4270-9f37-c609b48a9bb2'],
    ]);
    
    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'uuid_c, uuid2_c',
        'table' => 'test.uuid_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.uuid_test'
        ]);
    }
}

?>
--EXPECT--
array(4) {
  [0]=>
  array(2) {
    ["uuid_c"]=>
    string(32) "31249a1b7b0542709f37c609b48a9bb2"
    ["uuid2_c"]=>
    string(32) "31249a1b7b0542709f37c609b48a9bb2"
  }
  [1]=>
  array(2) {
    ["uuid_c"]=>
    string(32) "31249a1b7b0542709f37c609b48a9bb2"
    ["uuid2_c"]=>
    string(32) "00000000000000000000000000000000"
  }
  [2]=>
  array(2) {
    ["uuid_c"]=>
    string(32) "00000000000000009f37c609b48a9bb2"
    ["uuid2_c"]=>
    string(32) "00000000000000000000000000000000"
  }
  [3]=>
  array(2) {
    ["uuid_c"]=>
    string(32) "31249a1b7b0542709f37c609b48a9bb2"
    ["uuid2_c"]=>
    string(32) "00000000000000000000000000000000"
  }
}
