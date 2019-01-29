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

    testString($client, $deleteTable);
}

function testString($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.string_test (string_c String, fixedstring_c FixedString(50)) ENGINE = Memory");

    $client->insert("test.string_test", [
        'string_c', 'fixedstring_c'
    ], [
        [
            'string_c1',
            'fixedstring_c1'
        ],
        [
            'string_c2',
            'fixedstring_c2'
        ]
    ]);
    
    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'string_c, fixedstring_c',
        'table' => 'test.string_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.string_test'
        ]);
    }
}

?>
--EXPECT--
array(2) {
  [0]=>
  array(2) {
    ["string_c"]=>
    string(9) "string_c1"
    ["fixedstring_c"]=>
    string(14) "fixedstring_c1"
  }
  [1]=>
  array(2) {
    ["string_c"]=>
    string(9) "string_c2"
    ["fixedstring_c"]=>
    string(14) "fixedstring_c2"
  }
}
