<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

//Creating Array for JSON response
$response = array();
 
$DB_HOST = "localhost";
$DB_USER = "id18004373_khaled";
$DB_PASS = "2wad@Accentdb";
$DB_NAME = "id18004373_accentdb";

$con=mysqli_connect($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

if (mysqli_connect_errno($con)) {
   // If Connection failed
    $response["status"] = 0;
    $response["message"] =  "Failed to connect to MySQL: " . mysqli_connect_error();
 
    // Show JSON response
    echo json_encode($response);
}
else
{
    // Check if we got the field from the user
    if (isset($_GET['time']) && isset($_GET['lat']) && isset($_GET['lon']) && isset($_GET['state'])) 
    {
        $time = $_GET['time'];
        $lat= $_GET['lat'];
        $lon = $_GET['lon'];
        $state = $_GET['state'];
        
        $result = mysqli_query($con, "SELECT * FROM gps");
    	$id = mysqli_num_rows($result);
    	$id = $id + 1;
    	
    	$result = mysqli_query($con,"INSERT INTO gps (id, Time, lat, lon, state) VALUES ('$id', $time, '$lat', '$lon', $state)");
     
        // Check for succesfull execution of query and no results found
        if ($result) 
        {
            // successfully updation of LED status (status)
            $response["status"] = 1;
            $response["message"] = "data updated.";
     
            // Show JSON response
            echo json_encode($response);
        } 
        else 
        {
            $response["status"] = 0;
            $response["message"] = "failed to update";
         
            // Show JSON response
            echo json_encode($response);
        }
    } 
    else 
    {
        // If required parameter is missing
        $response["status"] = 0;
        $response["message"] = "Parameter(s) are missing. Please check the request";
     
        // Show JSON response
        echo json_encode($response);
    }
}
?>