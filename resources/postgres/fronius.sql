\c fronius

DROP MATERIALIZED VIEW IF EXISTS "monthly_view" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "daily_view" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "cagg_power" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "cagg_daily" CASCADE;
DROP TABLE IF EXISTS "archive" CASCADE;
DROP TABLE IF EXISTS "live" CASCADE;
DROP TABLE IF EXISTS "sensors" CASCADE;
DROP TABLE IF EXISTS "plan" CASCADE;
DROP TABLE IF EXISTS "state" CASCADE;
DROP TABLE IF EXISTS "code" CASCADE;

CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TABLE "sensors" (
  id SERIAL PRIMARY KEY,
  mfg VARCHAR(50),
  model VARCHAR(50),
  serial VARCHAR(50),
  firmware VARCHAR(50),
  power_max INTEGER
);

CREATE TABLE "plan" (
  id SERIAL PRIMARY KEY,
  payment DOUBLE PRECISION
);

CREATE TABLE "code" (
  id INTEGER NOT NULL,
  model VARCHAR(50),
  description VARCHAR(255)
);

CREATE TABLE "state" (
  id SERIAL PRIMARY KEY,
  description VARCHAR(255)
);

CREATE TABLE "live" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  plan_id INTEGER NOT NULL,
  state INTEGER,
  code INTEGER,
  ac_current DOUBLE PRECISION,
  ac_voltage DOUBLE PRECISION,
  ac_power_w DOUBLE PRECISION,
  ac_power_va DOUBLE PRECISION,
  ac_power_var DOUBLE PRECISION,
  ac_pf DOUBLE PRECISION,
  ac_freq DOUBLE PRECISION,
  ac_energy DOUBLE PRECISION,
  dc_voltage_1 DOUBLE PRECISION,
  dc_current_1 DOUBLE PRECISION,
  dc_power_1 DOUBLE PRECISION,
  dc_energy_1 DOUBLE PRECISION,
  dc_voltage_2 DOUBLE PRECISION,
  dc_current_2 DOUBLE PRECISION,
  dc_power_2 DOUBLE PRECISION,
  dc_energy_2 DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id),
  CONSTRAINT plan_id FOREIGN KEY (plan_id) REFERENCES plan (id),
  CONSTRAINT state FOREIGN KEY (state) REFERENCES state (id)
  ---CONSTRAINT code FOREIGN KEY (code) REFERENCES code (id)
);

SELECT create_hypertable('live', 'time');
SELECT add_retention_policy('live', INTERVAL '30 days');

GRANT INSERT, SELECT ON TABLE live TO nodejs;
GRANT SELECT ON TABLE live TO grafana;

GRANT SELECT ON TABLE sensors TO nodejs;
GRANT SELECT ON TABLE sensors TO grafana;

GRANT SELECT ON TABLE plan TO nodejs;
GRANT SELECT ON TABLE plan TO grafana;
