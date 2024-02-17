create or replace function get_solar_trend(
	out energy_percent float,
	out credit_percent float)
language plpgsql
as
$$
declare
begin
	drop table if exists temp_trend;
	
	-- initialize variables
	energy_percent := 0;
	credit_percent := 0;
	
	-- calculate model energy, credit up to yesterday
    CREATE TEMP TABLE temp_trend
	AS
	SELECT
	  date_trunc('year', now()) AS time, 
	  sum(energy) as energy, 
	  sum(energy * payment) as credit
	FROM model JOIN plan ON model.plan_id = plan.id
	where
	  doy < EXTRACT('doy' FROM now() - INTERVAL '1 day');

	-- calculate energy and credit for current year up to yesterday
	insert into temp_trend (time, energy, credit)
	select
      date_trunc('year', now()) AS time,
      sum(energy) as energy,
      sum(credit) as credit
    from daily_view
    where
	  time between date_trunc('year', now()) AND now() - INTERVAL '1 day';
	
	-- calculate percentages
    select (lead(energy) over (order by time) - energy) / nullif(energy, 0) * 100 from temp_trend into energy_percent;
	select (lead(credit) over (order by time) - credit) / nullif(credit, 0) * 100 from temp_trend into credit_percent;
end;
$$
