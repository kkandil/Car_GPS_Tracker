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
	$response["status"] = -1;
	$response["message"] =  "Failed to connect to MySQL: " . mysqli_connect_error();

	// Show JSON response
	echo json_encode($response);
}
else
{
	if (isset($_GET['Parm']) && isset($_GET['Value']) )
	{
		$Parm = $_GET['Parm']; 
		$Value = $_GET['Value']; 
		$result = mysqli_query($con,"UPDATE Accent SET Value=$Value WHERE Parm=$Parm");
		
		if ($result)  {
		  echo "Record updated successfully";
		} else {
		  echo "Error updating record: " . $conn->error;
		}
	} 
	mysqli_close($con);
} 
?>