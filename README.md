# Project BeeHive

## API server

> Hive units sends json data to the server, which is written in PHP at [php-server](php-server/).

## Guide

1. Insert DB authentication details to [config.php](php-server/src/config.php) in the following format.
   Make sure the DB contains tables specified in [sql](php-server/sql/).
2. Upload contents of the [src](php-server/src/) to a hosting provider.

config.php

```php
<?php

// Database configuration
$servername = "server.com";
$username = "user";
$password = "password";
$database = "database";
$port = 3306;

$conn = new mysqli($servername, $username, $password, $database, $port);
?>
```

## Hive unit

> Hive units are configured using [PlatformIO](https://platformio.org/)
> in [arduino-megaatmega2560](arduino-megaatmega2560/).

### Guide

1. Create a record with a secure `api_key` in the `hive_units` table of the PHP server DB.
2. Insert following keys to the [secrets.env](arduino-megaatmega2560/secrets.env)
    - API_KEY=**Alphanumerical** key created above.
    - API_SERVER=Php server domain or ip.
    - API_PATH=Path to the endpoint. if the php server is not nested it will be `/`.
    - API_PORT=Server port to make the request to.
3. Insert the API endpoint details to the [main.cpp][arduino-megaatmega2560/src/main.cpp]
4. Connect the hive unit's arduino board to the computer.
5. Open a terminal inside [arduino-megaatmega2560](arduino-megaatmega2560/).
6. Enter the command `upload`.
7. Enter `calibrate` and follow the instructions on the serial monitor.
8. Re-enter the command `upload` in the terminal.
9. Enter `release`.
