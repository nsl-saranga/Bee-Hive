CREATE TABLE hive_units
(
    id          INT AUTO_INCREMENT PRIMARY KEY,
    api_key     CHAR(25) NOT NULL UNIQUE,
    description TEXT,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
