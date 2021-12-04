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

$filename = 'google_coordinates.kml';
if( $num_fields > 0 )
{
    if (file_exists($filename)) 
    {
        unlink($filename);
    }
   
    $handle = fopen($filename, 'w+'); 
    
    fwrite($handle, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fwrite($handle, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    fwrite($handle, "<Folder>\n"); 
    for ($i=0; $i < $num_fields; $i++) { 
        $arr = mysqli_fetch_array($result);
        
        fwrite($handle, "\t<Placemark>\n");
        $line = sprintf("\t\t<name>P%s</name>\n", $arr["id"]);
        fwrite($handle, $line);
        $line = sprintf("\t\t<description>%s: %s</description>\n", $arr["Time"], $arr["state"]);
        fwrite($handle, $line);
        
        fwrite($handle, "\t\t<Point>\n");
        $line = sprintf("\t\t\t<coordinates>%.06f,%.06f</coordinates>\n", $arr["lon"], $arr["lat"]);
        fwrite($handle, $line);
        fwrite($handle, "\t\t</Point>\n");
        fwrite($handle, "\t</Placemark>\n");
    }
    fwrite($handle, "</Folder>\n"); 
    fwrite($handle, "</kml>\n"); 
    fclose($handle);
    
}
mysqli_close($con);

?>