--TEST--
SeasClick Single Column Fetch
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

$client->execute("CREATE TABLE IF NOT EXISTS test.single_val_fetch (
	tuple_c Tuple(id UInt64, name String),
	int64_c UInt64,
	string_c String,
	array_c Array(Int8),
	arraynull_c Array(Nullable(String)),
	enum8_c Enum8('One8' = 1, 'Two8' = 2),
	enum16_c Enum16('One16' = 1, 'Two16' = 2),
	fixedstring_c FixedString(50),
	int8null_c Nullable(Int8),
	stringnull_c Nullable(String),
	enumnull_c Nullable(Enum8('One8' = 1, 'Two8' = 2)),
	float32null_c Nullable(Float32),
	uuidnull_c Nullable(UUID),
	int8_c Int8,
	int16_c Int16,
	uint8_c UInt8,
	uint16_c UInt16,
	float32_c Float32,
	float64_c Float64,
	uuid_c UUID,
	uuid2_c UUID,
	date_c Date,
	datetime_c DateTime
) ENGINE = Memory");

$data = [
    [
        'tuple_c'       => [1, 'one'],
        'int64_c'       => 1,
        'string_c'      => 'string_one',
        'array_c'       => [1, 2, 3],
        'arraynull_c'   => ['str_array'],
        'enum8_c'       => 1,
        'enum16_c'      => 'Two16',
        'fixedstring_c' => 'fixedstring_c1',
        'int8null_c'    => 8,
        'stringnull_c'  => 'string',
        'enumnull_c'    => 'One8',
        'float32null_c' => 7.77,
        'uuidnull_c'    => '31249a1b7b0542709f37c609b48a9bb2',
        'int8_c'        => 8,
        'int16_c'       => 16,
        'uint8_c'       => 18,
        'uint16_c'      => 20,
        'float32_c'     => 32.32,
        'float64_c'     => 64.64,
        'uuid_c'        => '31249a1b-7b05-4270-9f37-c609b48a9bb2',
        'uuid2_c'       => '31249a1b7b0542709f37c609b48a9bb2',
        'date_c'        => 1548633600,
        'datetime_c'    => 1548687925,
    ],
    [
        'tuple_c'       => [2, 'two'],
        'int64_c'       => 2,
        'string_c'      => 'string_two',
        'array_c'       => [2, 3, 4],
        'arraynull_c'   => [null],
        'enum8_c'       => 'Two8',
        'enum16_c'      => 2,
        'fixedstring_c' => 'fixedstring_c2',
        'int8null_c'    => null,
        'stringnull_c'  => null,
        'enumnull_c'    => null,
        'float32null_c' => null,
        'uuidnull_c'    => null,
        'int8_c'        => 28,
        'int16_c'       => 216,
        'uint8_c'       => 218,
        'uint16_c'      => 220,
        'float32_c'     => 232.32,
        'float64_c'     => 264.64,
        'uuid_c'        => '31249a1b-7b05-4270-9f37-c609b48a9bb2',
        'uuid2_c'       => null,
        'date_c'        => 1548547200,
        'datetime_c'    => 1548513600,
    ],
];

$fields = array_keys(current($data));

$expected = $data;
$expected[0]['uuid_c'] = '31249a1b7b0542709f37c609b48a9bb2';
$expected[0]['enum8_c'] = 'One8';
$expected[1]['enum16_c'] = 'Two16';
$expected[1]['uuid_c'] = '31249a1b7b0542709f37c609b48a9bb2';
$expected[1]['arraynull_c'] = [null];
$expected[1]['uuid2_c'] = '00000000000000000000000000000000';

$client->insert('test.single_val_fetch', $fields, [array_values($data[0]), array_values($data[1])]);

foreach ($fields as $field) {
    $result = $client->select("SELECT {$field} FROM test.single_val_fetch ORDER BY int64_c ASC", [], SeasClick::FETCH_COLUMN);

    $res = var_export($result, true);
    $exp = var_export([$expected[0][$field], $expected[1][$field]], true);
    $match = $res === $exp ? 'OK' : 'FAIL';

    echo $field, ': ', $res , ' - ', $exp , ' - ', $match, "\n";
}

$client->execute('DROP TABLE test.single_val_fetch');
?>
--EXPECT--
tuple_c: array (
  0 => 
  array (
    0 => 1,
    1 => 'one',
  ),
  1 => 
  array (
    0 => 2,
    1 => 'two',
  ),
) - array (
  0 => 
  array (
    0 => 1,
    1 => 'one',
  ),
  1 => 
  array (
    0 => 2,
    1 => 'two',
  ),
) - OK
int64_c: array (
  0 => 1,
  1 => 2,
) - array (
  0 => 1,
  1 => 2,
) - OK
string_c: array (
  0 => 'string_one',
  1 => 'string_two',
) - array (
  0 => 'string_one',
  1 => 'string_two',
) - OK
array_c: array (
  0 => 
  array (
    0 => 1,
    1 => 2,
    2 => 3,
  ),
  1 => 
  array (
    0 => 2,
    1 => 3,
    2 => 4,
  ),
) - array (
  0 => 
  array (
    0 => 1,
    1 => 2,
    2 => 3,
  ),
  1 => 
  array (
    0 => 2,
    1 => 3,
    2 => 4,
  ),
) - OK
arraynull_c: array (
  0 => 
  array (
    0 => 'str_array',
  ),
  1 => 
  array (
    0 => NULL,
  ),
) - array (
  0 => 
  array (
    0 => 'str_array',
  ),
  1 => 
  array (
    0 => NULL,
  ),
) - OK
enum8_c: array (
  0 => 'One8',
  1 => 'Two8',
) - array (
  0 => 'One8',
  1 => 'Two8',
) - OK
enum16_c: array (
  0 => 'Two16',
  1 => 'Two16',
) - array (
  0 => 'Two16',
  1 => 'Two16',
) - OK
fixedstring_c: array (
  0 => 'fixedstring_c1',
  1 => 'fixedstring_c2',
) - array (
  0 => 'fixedstring_c1',
  1 => 'fixedstring_c2',
) - OK
int8null_c: array (
  0 => 8,
  1 => NULL,
) - array (
  0 => 8,
  1 => NULL,
) - OK
stringnull_c: array (
  0 => 'string',
  1 => NULL,
) - array (
  0 => 'string',
  1 => NULL,
) - OK
enumnull_c: array (
  0 => 'One8',
  1 => NULL,
) - array (
  0 => 'One8',
  1 => NULL,
) - OK
float32null_c: array (
  0 => 7.77,
  1 => NULL,
) - array (
  0 => 7.77,
  1 => NULL,
) - OK
uuidnull_c: array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => NULL,
) - array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => NULL,
) - OK
int8_c: array (
  0 => 8,
  1 => 28,
) - array (
  0 => 8,
  1 => 28,
) - OK
int16_c: array (
  0 => 16,
  1 => 216,
) - array (
  0 => 16,
  1 => 216,
) - OK
uint8_c: array (
  0 => 18,
  1 => 218,
) - array (
  0 => 18,
  1 => 218,
) - OK
uint16_c: array (
  0 => 20,
  1 => 220,
) - array (
  0 => 20,
  1 => 220,
) - OK
float32_c: array (
  0 => 32.32,
  1 => 232.32,
) - array (
  0 => 32.32,
  1 => 232.32,
) - OK
float64_c: array (
  0 => 64.64,
  1 => 264.64,
) - array (
  0 => 64.64,
  1 => 264.64,
) - OK
uuid_c: array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => '31249a1b7b0542709f37c609b48a9bb2',
) - array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => '31249a1b7b0542709f37c609b48a9bb2',
) - OK
uuid2_c: array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => '00000000000000000000000000000000',
) - array (
  0 => '31249a1b7b0542709f37c609b48a9bb2',
  1 => '00000000000000000000000000000000',
) - OK
date_c: array (
  0 => 1548633600,
  1 => 1548547200,
) - array (
  0 => 1548633600,
  1 => 1548547200,
) - OK
datetime_c: array (
  0 => 1548687925,
  1 => 1548513600,
) - array (
  0 => 1548687925,
  1 => 1548513600,
) - OK