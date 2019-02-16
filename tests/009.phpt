--TEST--
SeasClick testDate
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

    testDate($client, $deleteTable);
}

function testDate($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.date_test (date_c Date, datetime_c DateTime) ENGINE = Memory");

    $time1 = strtotime(date('Y-m-d H:i:s', 1548633600));
    $time2 = strtotime(date('Y-m-d H:i:s', 1548687925));

    $client->insert("test.date_test", [
        'date_c', 'datetime_c'
    ], [
        [$time1, $time2],
        [$time1, $time2]
    ]);
    
    $result = $client->select("SELECT {select} FROM {table}", [
        'select' => 'date_c, datetime_c',
        'table' => 'test.date_test'
    ]);
    var_dump($result);
    
    if ($deleteTable) {
        $client->execute("DROP TABLE {table}", [
            'table' => 'test.date_test'
        ]);
    }
}

?>
--EXPECT--
array(2) {
  [0]=>
  array(2) {
    ["date_c"]=>
    int(1548633600)
    ["datetime_c"]=>
    int(1548687925)
  }
  [1]=>
  array(2) {
    ["date_c"]=>
    int(1548633600)
    ["datetime_c"]=>
    int(1548687925)
  }
}
