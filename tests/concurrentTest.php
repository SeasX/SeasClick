<?php
/**
 * @author ahhhh.wang@gmail.com
 * Date: 19/02/6 8:23
 */

$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('SeasClick')) {
	dl('SeasClick.' . PHP_SHLIB_SUFFIX);
}

$config = [
    "host" => "10.0.6.38",
    "port" => "9000",
    "compression" => true
];

$http = new Swoole\Http\Server("127.0.0.1", 9501);
$http->set(array(
    'worker_num' => 4,
    'backlog' => 128,
));

$http->on("request", function ($request, $response) use ($config) {
    clientTest($config);
    $response->end('true');
});

$http->start();

function clientTest($config)
{
    $deleteTable = false;
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
