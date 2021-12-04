<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");


$DB_HOST = "localhost";
$DB_USER = "id18004373_khaled";
$DB_PASS = "2wad@Accentdb";
$DB_NAME = "id18004373_accentdb";

$con = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

 
$tables = array();


$result = mysqli_query($con, "SELECT * FROM gps");
$num_fields = mysqli_num_rows($result);

$handle = fopen('data.csv', 'w+');

for ($i=0; $i < $num_fields; $i++) { 
    $arr = mysqli_fetch_array($result);
    fwrite($handle, $arr["lat"]);
    fwrite($handle, ",");
    fwrite($handle, $arr["lon"]);
    fwrite($handle, ",");
    fwrite($handle, $arr["state"]);
    fwrite($handle, "\n");
}
fclose($handle);
//$handle = fopen('data.txt', 'w+');
//fwrite($handle,"Khaled\n");
//fwrite($handle,"mohamed");
//fclose($handle);

//echo "success";

mysqli_close($con);

?>