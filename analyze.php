<?php

$data = json_decode(file_get_contents("result.json"));

print "Total Duration: $data->total_duration\n";

$failed = 0;
$success = 0;
foreach($data->connection_stats as $foo) {
    if ($foo->failed) {
        $failed++;
    } else {
        $success++;
    }
}
print "Total failed: $failed\n";
print "Total succeeded: $success\n";



?>
