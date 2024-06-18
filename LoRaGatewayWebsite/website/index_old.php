<html>
	<head>
		<title>Sensors overview</title>
		<link rel="stylesheet" href="main.css">
	</head>
		<body>
			<div id="header">
				<p>
					<a href="/pinouts.php">Pinouts</a>
				</p>
				<p>
					<a href="/connections.php">Connections</a>
				</p>
				<p>
					<a href="/createTables.php">Initialise Database Tables</a>
				</p>
			</div>
			<div id="body">
				<div id="menu">
					<p>
						<button type="button">Add Sensor</button>
						<button type="button">Remove Sensor</button>
						<button type="button">Initialize Database</button>
					</p>
				</div>
				<div id="deviceList">
					<div id="listHeader">
						<span>List of all Sensors</span>
					</div>
<?php
	/*// Connect to database
	$dbhost = 'localhost';
	$dbuser = 'admin';
	$dbpass = '2chw4rz322ch4ff22';
	$dbname = 'lora_gateway';
	$conn = new mysqli($dbhost, $dbuser, $dbpass, $dbname);

	// Check connection
	if (!$conn->connect_error) {
		//get the datatypes
		$deviceTypeNames = [];
		$deviceTypeList = [];
		$selectQueryDevices = "SELECT * FROM DeviceType";
		$result = $conn->query($selectQueryDevices);
		if ($result->num_rows > 0) {
			while($row = $result->fetch_assoc()) {
				$deviceTypeNames[] = $row["deviceType"] => $row["deviceTypeName"];
				$deviceTypeList[] = $row["deviceType"] => $row["typeList"];
			}
		} else echo '<p>NO CONTENT</p>';
		
		print_r($deviceTypeNames);
		print_r($deviceTypeList);
		echo '<p>test2</p>';
		echo '<p>' . $deviceTypeNames[0] . '</p>';
		echo '<p>test3</p>';
		echo '<p>' . $deviceTypeList[1] . '</p>';
		
		// Get complete device list
		/*$selectQueryDevices = "SELECT * FROM Devices";
		if ($result = $conn->query($selectQueryDevices) === TRUE) {
			if ($result->num_rows > 0) {
				// output data of each row
				while($row = $result->fetch_assoc()) {
					echo '<div class="device" id="' . $row["deviceId"] . '">';
					//TODO add the device parameter
					echo '<button type="button">Set config</button>'
					echo '</div>';
				}
			} else {
				echo "0 results";
			}
		} else {
		  echo '<div class="errorMessage">Error getting device list: ' . $conn->error . '</div>';
		}/
	} else {
		echo '<div class="errorMessage">Connection failed: ' . $conn->connect_error;
	}*/
	$message = 'Hello World!';
	echo '<div class="helloWorld">' . $message . '</div>';
?>
				</div>
			</div>
		</body>
	</html>