/* TODO: on error condition! */
select count(*) from NoSuchTable;
/* Fail quickly if there's no Simulate_RaceChart table. */
select count(*) from Simulate_RaceChart;
/*
delete from RaceChart;

insert into RaceChart select * from Simulate_RaceChart;
*/