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

    testTuple($client, $deleteTable);
}

function testTuple($client, $deleteTable = false) {
    $result = $client->select("SELECT tuple(1,'a', 5, 'b') AS a");
    var_dump($result);
}

?>
--EXPECT--
array(1) {
  [0]=>
  array(1) {
    ["a"]=>
    array(4) {
      [0]=>
      int(1)
      [1]=>
      string(1) "a"
      [2]=>
      int(5)
      [3]=>
      string(1) "b"
    }
  }
}
