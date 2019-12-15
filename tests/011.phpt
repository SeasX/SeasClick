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
    for ($i = 1; $i < 3; $i++) {
        $result = $client->select("SELECT {$field} FROM test.single_val_fetch WHERE int64_c = {i}", ['i' => $i], SeasClick::FETCH_ONE);
        $match = $result === $expected[$i-1][$field] ? 'OK' : 'FAIL';
        echo $field, ': ', var_export($result, true), ' - ', var_export($expected[$i - 1][$field], true), ' - ', $match, "\n";
    }
}

$client->execute('DROP TABLE test.single_val_fetch');
?>
--EXPECT--
tuple_c: array (
  0 => 1,
  1 => 'one',
) - array (
  0 => 1,
  1 => 'one',
) - OK
tuple_c: array (
  0 => 2,
  1 => 'two',
) - array (
  0 => 2,
  1 => 'two',
) - OK
int64_c: 1 - 1 - OK
int64_c: 2 - 2 - OK
string_c: 'string_one' - 'string_one' - OK
string_c: 'string_two' - 'string_two' - OK
array_c: array (
  0 => 1,
  1 => 2,
  2 => 3,
) - array (
  0 => 1,
  1 => 2,
  2 => 3,
) - OK
array_c: array (
  0 => 2,
  1 => 3,
  2 => 4,
) - array (
  0 => 2,
  1 => 3,
  2 => 4,
) - OK
arraynull_c: array (
  0 => 'str_array',
) - array (
  0 => 'str_array',
) - OK
arraynull_c: array (
  0 => NULL,
) - array (
  0 => NULL,
) - OK
enum8_c: 'One8' - 'One8' - OK
enum8_c: 'Two8' - 'Two8' - OK
enum16_c: 'Two16' - 'Two16' - OK
enum16_c: 'Two16' - 'Two16' - OK
fixedstring_c: 'fixedstring_c1' - 'fixedstring_c1' - OK
fixedstring_c: 'fixedstring_c2' - 'fixedstring_c2' - OK
int8null_c: 8 - 8 - OK
int8null_c: NULL - NULL - OK
stringnull_c: 'string' - 'string' - OK
stringnull_c: NULL - NULL - OK
enumnull_c: 'One8' - 'One8' - OK
enumnull_c: NULL - NULL - OK
float32null_c: 7.77 - 7.77 - OK
float32null_c: NULL - NULL - OK
uuidnull_c: '31249a1b7b0542709f37c609b48a9bb2' - '31249a1b7b0542709f37c609b48a9bb2' - OK
uuidnull_c: NULL - NULL - OK
int8_c: 8 - 8 - OK
int8_c: 28 - 28 - OK
int16_c: 16 - 16 - OK
int16_c: 216 - 216 - OK
uint8_c: 18 - 18 - OK
uint8_c: 218 - 218 - OK
uint16_c: 20 - 20 - OK
uint16_c: 220 - 220 - OK
float32_c: 32.32 - 32.32 - OK
float32_c: 232.32 - 232.32 - OK
float64_c: 64.64 - 64.64 - OK
float64_c: 264.64 - 264.64 - OK
uuid_c: '31249a1b7b0542709f37c609b48a9bb2' - '31249a1b7b0542709f37c609b48a9bb2' - OK
uuid_c: '31249a1b7b0542709f37c609b48a9bb2' - '31249a1b7b0542709f37c609b48a9bb2' - OK
uuid2_c: '31249a1b7b0542709f37c609b48a9bb2' - '31249a1b7b0542709f37c609b48a9bb2' - OK
uuid2_c: '00000000000000000000000000000000' - '00000000000000000000000000000000' - OK
date_c: 1548633600 - 1548633600 - OK
date_c: 1548547200 - 1548547200 - OK
datetime_c: 1548687925 - 1548687925 - OK
datetime_c: 1548513600 - 1548513600 - OK
