CREATE TABLE hive_data
(
    id         INT AUTO_INCREMENT PRIMARY KEY,
    unit_id    INT    NOT NULL,
    temp       DOUBLE NOT NULL,
    hum        DOUBLE NOT NULL,
    weight     DOUBLE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
