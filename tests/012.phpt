--TEST--
SeasClick Date Formatting
--SKIPIF--
<?php if (!extension_loaded("SeasClick")) print "skip"; ?>
--FILE--
<?php
$config = [
    "host"        => "clickhouse",
    "port"        => "9000",
    "compression" => true,
];

$deleteTable = true;
$client = new SeasClick($config);
$client->execute('CREATE DATABASE IF NOT EXISTS test');

$client->execute("CREATE TABLE IF NOT EXISTS test.dates (
	date_c Date,
	datetime_c DateTime
) ENGINE = Memory");

$data = [
    [
        'date_c'        => 1548633600,
        'datetime_c'    => 1548687925,
    ],
    [
        'date_c'        => 1548547200,
        'datetime_c'    => 1548513600,
    ],
];
$expected = [
    [
            'date_c'        => date('Y-m-d', 1548633600),
            'datetime_c'    => date('Y-m-d H:i:s', 1548687925),
        ],
        [
            'date_c'        => date('Y-m-d', 1548547200),
            'datetime_c'    => date('Y-m-d H:i:s', 1548513600),
        ],
];

$fields = array_keys(current($data));
$client->insert('test.dates', $fields, [array_values($data[0]), array_values($data[1])]);

$res = $client->select("SELECT * FROM test.dates", [], SeasClick::DATE_AS_STRINGS);
var_dump($res);
if (array_diff_assoc($expected[0], $res[0]) || array_diff_assoc($expected[1], $res[1])) {
    echo "FAIL\n";
} else {
    echo "OK\n";
}

$res = $client->select("SELECT date_c FROM test.dates WHERE datetime_c = 1548687925", [], SeasClick::FETCH_ONE|SeasClick::DATE_AS_STRINGS);
echo $res, ' = ', $expected[0]['date_c'], ' ', ($res === $expected[0]['date_c'] ? 'OK' : 'FAIL'), "\n";

$res = $client->select("SELECT datetime_c FROM test.dates WHERE datetime_c = 1548687925", [], SeasClick::FETCH_ONE|SeasClick::DATE_AS_STRINGS);
echo $res, ' = ', $expected[0]['datetime_c'], ' ' , ($res === $expected[0]['datetime_c'] ? 'OK' : 'FAIL'), "\n";

$client->execute('DROP TABLE test.dates');
?>
--EXPECT--
array(2) {
  [0]=>
  array(2) {
    ["date_c"]=>
    string(10) "2019-01-27"
    ["datetime_c"]=>
    string(19) "2019-01-28 10:05:25"
  }
  [1]=>
  array(2) {
    ["date_c"]=>
    string(10) "2019-01-26"
    ["datetime_c"]=>
    string(19) "2019-01-26 09:40:00"
  }
}
OK
2019-01-27 = 2019-01-27 OK
2019-01-28 10:05:25 = 2019-01-28 10:05:25 OK
