<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

$DB_HOST = "localhost";
$DB_USER = "id18004373_khaled";
$DB_PASS = "2wad@Accentdb";
$DB_NAME = "id18004373_accentdb";

$con=mysqli_connect($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

if (mysqli_connect_errno($con)) {
    echo "Failed to connect to MySQL: " . mysqli_connect_error();
}

$response = array();
if (isset($_GET["id"])) 
{
    $id = $_GET['id'];
    #$username = $_POST['username'];
    #$password = $_POST['password']; 
    $result = mysqli_query($con,"SELECT *FROM gps WHERE id = '$id'");
    #echo $result;
    $row = mysqli_fetch_array($result);
    
    // temperoary user array
    $DeviceData = array();
    $DeviceData["lat"] = $row["lat"];
    $DeviceData["lon"] = $row["lon"];
    $DeviceData["state"] = $row["state"];
    $response["success"] = 1;
    
    $response["DeviceData"] = array();
    
    // Push all the items 
    array_push($response["DeviceData"], $DeviceData);
    
    // Show JSON response
    echo json_encode($response);
}

mysqli_close($con);
?>