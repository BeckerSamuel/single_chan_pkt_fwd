<html>
	<head>
		<title>Initialise LoRa Gateway Database</title>
		<link rel="stylesheet" href="main.css">
	</head>
	<body>
<?php
	echo '<div>Hello World!</div>';

	// Connect to database
	$dbhost = 'localhost';
	$dbuser = 'admin';
	$dbpass = '2chw4rz322ch4ff22';
	$conn = new mysqli($dbhost, $dbuser, $dbpass);
	
	// Check connection
	if (!$conn->connect_error) {
		echo '<div>Connected to Database: ' .  $conn->host_info . '</div>';
	} else {
		die("Connection failed: " . $conn->connect_error);
	}
	
	// Create database
	$sql = "CREATE DATABASE IF NOT EXISTS lora_gateway";
	if ($conn->query($sql) === TRUE) {
		echo '<div>Created Database "lora_gateway"</div>';
	} else {
		die("Error creating database: " . $conn->error);
	}
	
	// Select to new database
	$dbname = "lora_gateway";
	$conn->select_db($dbname);
	
	// #########################################################
	// ######################## Devices ########################
	// #########################################################
	$tableDevices = "CREATE TABLE IF NOT EXISTS Devices (
	deviceId INT(8) UNSIGNED PRIMARY KEY NOT NULL,
	deviceType TINYINT(1) UNSIGNED NOT NULL,
	deviceName VARCHAR(50) NOT NULL
	)";

	if ($conn->query($tableDevices) === TRUE) {
	  echo '<div>Table Devices created successfully</div>';
	} else {
	  echo '<div>Error creating table Devices: ' . $conn->error . '</div>';
	}
	
	// #########################################################
	// ######################## DeviceType ########################
	// #########################################################
	$tableDeviceType = "CREATE TABLE IF NOT EXISTS DeviceType (
	deviceType TINYINT(1) UNSIGNED PRIMARY KEY NOT NULL,
	deviceTypeName VARCHAR(50) NOT NULL,
	typeList VARCHAR(100) NOT NULL
	)";

	if ($conn->query($tableDeviceType) === TRUE) {
	  echo '<div>Table DeviceType created successfully</div>';
	} else {
	  echo '<div>Error creating table DeviceType: ' . $conn->error . '</div>';
	}
	
	// #########################################################
	// ###################### DeviceType00 #####################
	// #########################################################
	$tableDeviceType00 = "CREATE TABLE IF NOT EXISTS DeviceType00 (
	uniqueId INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	deviceId INT(8) UNSIGNED NOT NULL,
	timestamp TIMESTAMP NOT NULL,
	message VARCHAR(255) NOT NULL
	)";

	if ($conn->query($tableDeviceType00) === TRUE) {
	  echo '<div>Table DeviceType00 created successfully</div>';
	} else {
	  echo 'Error creating table DeviceType00: ' . $conn->error . '</div>';
	}
	
	// #########################################################
	// ###################### DeviceType01 #####################
	// #########################################################
	$tableDeviceType01 = "CREATE TABLE IF NOT EXISTS DeviceType01 (
	uniqueId INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	deviceId INT(8) UNSIGNED NOT NULL,
	timestamp TIMESTAMP NOT NULL,
	temperature TINYINT(1),
	humidity TINYINT(1) UNSIGNED,
	lightLevel SMALLINT (2) UNSIGNED,
	rainLevel SMALLINT (2) UNSIGNED
	)";

	if ($conn->query($tableDeviceType01) === TRUE) {
	  echo '<div>Table DeviceType01 created successfully</div>';
	} else {
	  echo '<div>Error creating table DeviceType01: ' . $conn->error . '</div>';
	}
	
	// #########################################################
	// ###################### DeviceConfig #####################
	// #########################################################
	$tableDeviceConfig = "CREATE TABLE IF NOT EXISTS DeviceConfig (
	deviceId INT(8) UNSIGNED PRIMARY KEY NOT NULL,
	deviceType TINYINT(1) NOT NULL,
	deviceConfig VARCHAR(100) NOT NULL,
	deviceConfigSend BIT(1) NOT NULL DEFAULT 1
	)";

	if ($conn->query($tableDeviceConfig) === TRUE) {
	  echo 'Table DeviceConfig created successfully</div>';
	} else {
	  echo '<div>Error creating table DeviceConfig: ' . $conn->error . '</div>';
	}
	
	$conn->close();
?>
	</body>
</html>