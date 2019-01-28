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

    testArray($client, $deleteTable);
}

function testArray($client, $deleteTable = false) {
    $client->execute("CREATE TABLE IF NOT EXISTS test.array_test (string_c String, array_c Array(Int8), arraynull_c Array(Nullable(String))) ENGINE = Memory");

    $client->insert("test.array_test", [
        'string_c', 'array_c', 'arraynull_c'
    ], [
        [
            'string_c1', [1, 2, 3], ['string']
        ],
        [
            'string_c2', [4, 5, 6], [null]
        ]
    ]);

    $client->insert("test.array_test", [
        'string_c'
    ], [
        [
            'string_c1'
        ],
        [
            'string_c2'
        ]
    ]);

    $client->insert("test.array_test", [
        'array_c'
    ], [
        [
            [1, 2, 3]
        ],
        [
            [4, 5, 6]
        ]
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

?>
--EXPECT--
array(6) {
  [0]=>
  array(3) {
    ["string_c"]=>
    string(9) "string_c1"
    ["array_c"]=>
    array(3) {
      [0]=>
      int(1)
      [1]=>
      int(2)
      [2]=>
      int(3)
    }
    ["arraynull_c"]=>
    array(1) {
      ["array"]=>
      string(6) "string"
    }
  }
  [1]=>
  array(3) {
    ["string_c"]=>
    string(9) "string_c2"
    ["array_c"]=>
    array(3) {
      [0]=>
      int(4)
      [1]=>
      int(5)
      [2]=>
      int(6)
    }
    ["arraynull_c"]=>
    array(1) {
      [0]=>
      NULL
    }
  }
  [2]=>
  array(3) {
    ["string_c"]=>
    string(9) "string_c1"
    ["array_c"]=>
    array(0) {
    }
    ["arraynull_c"]=>
    array(0) {
    }
  }
  [3]=>
  array(3) {
    ["string_c"]=>
    string(9) "string_c2"
    ["array_c"]=>
    array(0) {
    }
    ["arraynull_c"]=>
    array(0) {
    }
  }
  [4]=>
  array(3) {
    ["string_c"]=>
    string(0) ""
    ["array_c"]=>
    array(3) {
      [0]=>
      int(1)
      [1]=>
      int(2)
      [2]=>
      int(3)
    }
    ["arraynull_c"]=>
    array(0) {
    }
  }
  [5]=>
  array(3) {
    ["string_c"]=>
    string(0) ""
    ["array_c"]=>
    array(3) {
      [0]=>
      int(4)
      [1]=>
      int(5)
      [2]=>
      int(6)
    }
    ["arraynull_c"]=>
    array(0) {
    }
  }
}
