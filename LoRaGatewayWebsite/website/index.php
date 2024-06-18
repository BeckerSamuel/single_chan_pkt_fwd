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
<?php
						$message = 'Hello World!';
						echo '<div class="helloWorld"><span>' . $message . '</span></div>';
?>
					</div>
				</div>
			</div>
		</body>
	</html>