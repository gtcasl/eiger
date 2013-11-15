--
-- Eiger database schema
-- 

DROP TABLE IF EXISTS 
vectors,
matrices,
datacollections, 
executions,
trials, 
properties, 
machines, 
machine_metrics,
applications, 
datasets, 
metrics, 
deterministic_metrics, 
nondeterministic_metrics,
principal_component_analyses,
model_functions,
model_predictors,
models,
model_map;
	

--
-- Standard numeric types
--
CREATE TABLE vectors(
	ID int(11),
	vector longtext
);

CREATE TABLE matrices(
	ID int(11),
	matrix longtext
);

--
-- Tables to maintain a data collection
--
CREATE TABLE datacollections(
	ID int(11) auto_increment not null,
	name varchar(256),
	description text,
	created datetime,
  UNIQUE(name),
	PRIMARY KEY(ID)
);

CREATE TABLE trials(
	ID int(11) auto_increment not null,
	dataCollectionID int(11),
	machineID int(11),
	applicationID int(11),
	datasetID int(11),
	propertiesID int(11),
	PRIMARY KEY (ID),
	INDEX (datasetID)
);

CREATE TABLE executions(
	ID int(11) auto_increment not null,
	machineID int(11),
	trialID int(11),
	PRIMARY KEY (ID),
	INDEX (trialID)
);

CREATE TABLE properties(
	ID int(11) auto_increment not null,
	trialID int(11),
	propertyName text,
	property int(11),
	PRIMARY KEY(ID)
);

CREATE TABLE machines(
	ID int(11) auto_increment not null,
	name varchar(256),
	description text,
  UNIQUE(name),
	PRIMARY KEY (ID)
);

CREATE TABLE machine_metrics(
	machineID int(11),
	metricID int(11),
	metric real
);

CREATE TABLE applications(
	ID int(11) auto_increment not null,
	name varchar(256),
	description text,
  UNIQUE(name),
	PRIMARY KEY(ID)
);

CREATE TABLE datasets(
	ID int(11) auto_increment not null,
	applicationID int(11),
	name varchar(256),
	description text,
	created DATETIME,
	url text,
    UNIQUE(name),
	PRIMARY KEY (ID)
);

CREATE TABLE metrics(
	ID int(11) auto_increment not null,
	type enum('result', 'deterministic', 'nondeterministic', 'machine', 'other'),
	name varchar(256),
	description text,
  UNIQUE(name),
	PRIMARY KEY (ID)
);

-- Maps nondeterministic characteristics onto a trial.
CREATE TABLE nondeterministic_metrics(
	executionID int(11),
	metricID int(11),
	metric real,
	INDEX (executionID)
);

-- Maps deterministic characteristics onto a dataset. 
CREATE TABLE deterministic_metrics(
	datasetID int(11),
	metricID int(11),
	metric real,
	INDEX (datasetID)
);

	
--
-- Tables to serialized a performance model
--
CREATE TABLE principal_component_analyses(
	ID int(11) auto_increment not null,
	dataCollectionID int(11),
	machinePC longtext,
	machineWeight longtext,
	trainingPC longtext,
	trainingWeight longtext,
	PRIMARY KEY(ID)
);

CREATE TABLE models(
	ID int(11) auto_increment not null,
	dataCollectionID int(11),
	trainingPCs int(11),
	machinePCs int(11),
	regression varchar(256),
	PRIMARY KEY(ID)
);

CREATE TABLE model_functions(
	ID int(11) auto_increment not null,
	function varchar(256),
	UNIQUE(function),
	PRIMARY KEY(ID)
);

CREATE TABLE model_predictors(
	ID int(11) auto_increment not null,
	modelID int(11),
	beta real,
	PRIMARY KEY(ID)
);

CREATE TABLE model_map(
	predictorID int(11),
	functionID int(11)
);

/*
CREATE TABLE performance_models(
	ID int(11) auto_increment not null,
	model longtext,
	dataCollectionID int(11),
	PRIMARY KEY(ID)
);

CREATE TABLE input_matrix_columns(
	ID int(11) auto_increment not null,
	performanceModelID int(11),
	metricID int(11),
	PRIMARY KEY(ID)
);

CREATE TABLE design_matrix_columns(
	ID int(11),
	performanceModelID int(11),
	sort int(11),
	function text
);

CREATE TABLE design_matrix_map(
	sort int(11),
	designColumnID int(11),
	inputColumnID int(11)
);

CREATE TABLE training_data_map(
	performanceModelID int(11),
	dataCollectionID int(11)
);
*/

--
-- 
--

