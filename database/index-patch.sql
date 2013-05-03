create index trial_id_by_dataset_id on trials(datasetID);
create index exec_id_by_trial_id on executions(trialID);
create index detmetric_by_datasetid on deterministic_metrics(datasetID);
