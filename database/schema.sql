--
-- Eiger database schema
-- 

DROP TABLE IF EXISTS 
deterministic_metrics,
nondeterministic_metrics,
executions,
trials, 
machine_metrics,
metrics, 
machines, 
datasets, 
applications, 
datacollections;


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
    PRIMARY KEY (ID),
    FOREIGN KEY (applicationID) REFERENCES applications(ID)
);

CREATE TABLE machines(
    ID int(11) auto_increment not null,
    name varchar(256),
    description text,
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

CREATE TABLE machine_metrics(
    machineID int(11),
    metricID int(11),
    metric real,
    FOREIGN KEY (machineID) REFERENCES machines(ID),
    FOREIGN KEY (metricID) REFERENCES metrics(ID)
);

CREATE TABLE trials(
    ID int(11) auto_increment not null,
    dataCollectionID int(11),
    machineID int(11),
    applicationID int(11),
    datasetID int(11),
    PRIMARY KEY (ID),
    INDEX (datasetID),
    FOREIGN KEY (dataCollectionID) REFERENCES datacollections(ID),
    FOREIGN KEY (machineID) REFERENCES machines(ID),
    FOREIGN KEY (applicationID) REFERENCES applications(ID),
    FOREIGN KEY (datasetID) REFERENCES datasets(ID)
);

CREATE TABLE executions(
    ID int(11) auto_increment not null,
    machineID int(11),
    trialID int(11),
    PRIMARY KEY (ID),
    INDEX (trialID),
    FOREIGN KEY (machineID) REFERENCES machines(ID),
    FOREIGN KEY (trialID) REFERENCES trials(ID)
);

-- Maps nondeterministic characteristics onto a trial.
CREATE TABLE nondeterministic_metrics(
    executionID int(11),
    metricID int(11),
    metric real,
    INDEX (executionID),
    FOREIGN KEY (executionID) REFERENCES executions(ID),
    FOREIGN KEY (metricID) REFERENCES metrics(ID)
);

-- Maps deterministic characteristics onto a dataset. 
CREATE TABLE deterministic_metrics(
    datasetID int(11),
    metricID int(11),
    metric real,
    INDEX (datasetID),
    FOREIGN KEY (datasetID) REFERENCES datasets(ID),
    FOREIGN KEY (metricID) REFERENCES metrics(ID)
);

