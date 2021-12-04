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
	if (isset($_GET['AllParm']))
	{
		$result = mysqli_query($con, "SELECT * FROM Accent");
		$num_rows = mysqli_num_rows($result);
		if( $num_rows == 0 )
		{ 
			$response["status"] = -2;
			$response["message"] =  "Database was empty";

			// Show JSON response
			echo json_encode($response);

		}
		else
		{
			$AllParm = $_GET['AllParm'];
			if( $AllParm == 1 )
			{
				$Parameters = array();
			//	$response["Parameters"] = array();
				// return all parameters
				for ($i=0; $i < $num_rows; $i++) 
				{ 
					$arr = mysqli_fetch_array($result);
					$Parameters["ID"] = $arr["ID"];
					$Parameters["Parm"] = $arr["Parm"];
					$Parameters["Value"] = $arr["Value"];
					
					// Push all the items 
					array_push($response, $Parameters);
				}
				
				// Show JSON response
				echo json_encode($response);
			}
			else
			{
				// return only the requested parameter
				if (isset($_GET['Parm']))
	            {
	                $Parameter = array();
	                $Parm = $_GET['Parm']; 
	                $result = mysqli_query($con,"SELECT * FROM Accent WHERE Parm = $Parm");
	                $row = mysqli_fetch_array($result);
	                $Parameter["Parm"] = $row["Parm"];
	                $Parameter["Value"] = $row["Value"];
	                
	                array_push($response, $Parameter);
	                echo json_encode($response);
	            }
			}
		}
	}
	mysqli_close($con);
}





?>