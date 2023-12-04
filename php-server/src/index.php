<?php
include "config.php";

// used for API key validation
function get_hive_unit_id($api_key)
{
    $sql = "SELECT `id, api_key` FROM `hive_units`";
      $results = $conn->query($sql);
      while ($row = $results->fetch_assoc()) {
        if (hash_equals($row["api_key"], $api_key)) {
            return $row["id"];
        }
      }
      return null;
}

// API key from the request header
$api_key = $_SERVER['HTTP_API_KEY'];

// Validate API key and get the unit id
$unit_id = get_hive_unit_id($api_key);
if ($unit_id == NULL) {
    http_response_code(401);
    die('Unauthorized');
}

// Read JSON data from the request body
$data = json_decode(file_get_contents('php://input'), true);

// Validate JSON structure
$expected_keys = ['temp', 'hum', 'weight'];
if (!is_array($data) || count(array_diff($expected_keys, array_keys($data))) > 0) {
    http_response_code(400);
    die('Invalid JSON structure');
}

// explicitly define vars so that the insertion does not depend in the data order
$temp = $data["temp"];
$hum = $data["hum"];
$weight = $data["weight"];

// Insert data into the database
try {
    $sql = 'INSERT INTO hive_data (unit_id, temp, hum, weight) VALUES (?, ?, ?, ?)';
    $stmt = $conn->prepare($sql);
    // how values should be formatted when binding to the sql statement
    $types = 'iddd';
    $stmt->bind_param($types, $unit_id, $temp, $hum, $weight);
    $stmt->execute();
    $stmt->close();
    http_response_code(200);
    echo 'Data successfully stored';
} catch (Exception $e) {
    http_response_code(500);
    die('Error storing data: ' . $e->getMessage());
}

?>
