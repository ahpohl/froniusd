\c fronius

--
-- daily continuous aggregate
--
DROP MATERIALIZED VIEW IF EXISTS cagg_daily CASCADE;

CREATE MATERIALIZED VIEW cagg_daily
WITH (timescaledb.continuous, timescaledb.materialized_only=true) 
AS
SELECT
  time_bucket('1 day', time) AS bucket_1d,
  sensor_id,
  plan_id,
  last(ac_energy, time) - first(ac_energy, time) AS energy_1d,
  first(ac_energy, time) AS total,
  max(ac_power_w) AS power_max
FROM live
GROUP BY bucket_1d, plan_id, sensor_id
WITH NO DATA;

-- refresh policy
SELECT add_continuous_aggregate_policy('cagg_daily',
  start_offset => INTERVAL '3 days',
  end_offset => INTERVAL '1 hour',
  schedule_interval => '1 day');

--
-- average power
--
DROP MATERIALIZED VIEW IF EXISTS cagg_power CASCADE;

CREATE MATERIALIZED VIEW cagg_power
WITH (timescaledb.continuous, timescaledb.materialized_only=true)
AS
SELECT
  time_bucket('5 minutes', time) as bucket_5m,
  avg(ac_power_w) as ac_pwr,
  avg(dc_power_1) as dc_pwr_1,
  avg(dc_power_2) as dc_pwr_2
FROM live
GROUP BY bucket_5m           
WITH NO DATA;

-- refresh policy
SELECT add_continuous_aggregate_policy('cagg_power',
  start_offset => INTERVAL '3 hours',
  end_offset => INTERVAL '1 minute',
  schedule_interval => '5 minutes');

-- retention policy
SELECT add_retention_policy('cagg_power', INTERVAL '3 days');

-- grant
GRANT SELECT ON TABLE cagg_power TO grafana;
