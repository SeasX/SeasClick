<?php
/**
 * @author ahhhh.wang@gmail.com
 * Date: 19/02/6 8:23
 * This performance test demo is compared to phpclickhouse(https://github.com/smi2/phpClickHouse).
 */

include_once 'vendor/autoload.php';

// $dataCount, $seletCount, $limit
// edit this array
$testDataSet = [
    [10000, 1, 10000],
    [100, 500, 100],
    [100, 2000, 100],
];

foreach ($testDataSet as $key => $value) {
    list($dataCount, $seletCount, $limit) = $value;
    $insertData = initData($dataCount);

    echo "\n##### dataCount: {$dataCount}, seletCount: {$seletCount}, limit: {$limit} #####\n";

    $t0 = $t = start_test();
    testPhpClickhouse($insertData, $seletCount, $limit);
    $t = end_test($t, "PhpClickhouse");

    testSeasClickNonCompression($insertData, $seletCount, $limit);
    $t = end_test($t, "SeasClickNonCompression");

    testSeasClickCompression($insertData, $seletCount, $limit);
    $t = end_test($t, "SeasClickCompression");

    total($t0, "Total");
}

function start_test()
{
    return getmicrotime();
}

function getmicrotime()
{
    $t = gettimeofday();

    return ($t['sec'] + $t['usec'] / 1000000);
}

function end_test($start, $name)
{
    global $total;
    $end = getmicrotime();
    $total += $end - $start;
    $num = number_format($end - $start, 3);
    $pad = str_repeat(" ", 60 - strlen($name) - strlen($num));

    echo $name . $pad . $num . "\n";

    return getmicrotime();
}

function total()
{
    global $total;
    $pad = str_repeat("-", 32);
    echo $pad . "\n";
    $num = number_format($total, 3);
    $pad = str_repeat(" ", 32 - strlen("Total") - strlen($num));
    echo "Total" . $pad . $num . "\n";
}

function testSeasClickNonCompression($insertData, $num, $limit)
{
    $config = [
        "host" => "clickhouse",
        "port" => "9000",
        "compression" => false
    ];
    
    $db = new SeasClick($config);
    $db->execute("CREATE DATABASE IF NOT EXISTS test");

    $db->execute('
        CREATE TABLE IF NOT EXISTS test.summing_url_views (
            event_date Date DEFAULT toDate(event_time),
            event_time DateTime,
            site_id Int32,
            site_key String,
            views Int32,
            v_00 Int32,
            v_55 Int32
        )
        ENGINE = SummingMergeTree(event_date, (site_id, site_key, event_time, event_date), 8192)
    ');

    $db->insert("test.summing_url_views",
        ['event_time', 'site_key', 'site_id', 'views', 'v_00', 'v_55'],
        $insertData
    );

    $a = $num;
    while ($a--) {
        $db->select('SELECT * FROM test.summing_url_views LIMIT 100');
    }

    $db->execute("DROP TABLE {table}", [
        'table' => 'test.summing_url_views'
    ]);
}

function testSeasClickCompression($insertData, $num, $limit)
{
    $config = [
        "host" => "clickhouse",
        "port" => "9000",
        "compression" => true
    ];
    
    $db = new SeasClick($config);
    $db->execute("CREATE DATABASE IF NOT EXISTS test");

    $db->execute('
        CREATE TABLE IF NOT EXISTS test.summing_url_views (
            event_date Date DEFAULT toDate(event_time),
            event_time DateTime,
            site_id Int32,
            site_key String,
            views Int32,
            v_00 Int32,
            v_55 Int32
        )
        ENGINE = SummingMergeTree(event_date, (site_id, site_key, event_time, event_date), 8192)
    ');

    $db->insert("test.summing_url_views",
        ['event_time', 'site_key', 'site_id', 'views', 'v_00', 'v_55'],
        $insertData
    );

    $a = $num;
    while ($a--) {
        $db->select('SELECT * FROM test.summing_url_views LIMIT 100');
    }

    $db->execute("DROP TABLE {table}", [
        'table' => 'test.summing_url_views'
    ]);
}

function testPhpClickhouse($insertData, $num, $limit)
{
    $config = [
        'host' => 'clickhouse',
        'port' => '8123',
        'username' => 'default',
        'password' => ''
    ];
    $db = new ClickHouseDB\Client($config);
    $db->write("CREATE DATABASE IF NOT EXISTS test");

    $db->database('test');
    $db->setTimeout(1.5);      // 1500 ms
    $db->setTimeout(10);       // 10 seconds
    $db->setConnectTimeOut(5); // 5 seconds
    
    $db->write('
        CREATE TABLE IF NOT EXISTS summing_url_views (
            event_date Date DEFAULT toDate(event_time),
            event_time DateTime,
            site_id Int32,
            site_key String,
            views Int32,
            v_00 Int32,
            v_55 Int32
        )
        ENGINE = SummingMergeTree(event_date, (site_id, site_key, event_time, event_date), 8192)
    ');

    $a = 100;
    $insertData = [];
    while ($a--) {
        $insertData[] = [time(), 'HASH2', 2345, 12, 9,  3];
    }
    $db->insert("summing_url_views",
        $insertData,
        ['event_time', 'site_key', 'site_id', 'views', 'v_00', 'v_55']
    );

    $a = $num;
    while ($a--) {
        $db->select('SELECT * FROM summing_url_views LIMIT 100')->rows();
    }

    $db->write('DROP TABLE IF EXISTS summing_url_views');
}

function initData($num = 100)
{
    $insertData = [];
    while ($num--) {
        $insertData[] = [time(), 'HASH2', 2345, 12, 9,  3];
    }
    return $insertData;
}
