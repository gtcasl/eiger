--
-- Eiger database schema
-- 

DROP TABLE IF EXISTS deterministic_metrics;
DROP TABLE IF EXISTS nondeterministic_metrics;
DROP TABLE IF EXISTS trials;
DROP TABLE IF EXISTS machine_metrics;
DROP TABLE IF EXISTS metric_types;
DROP TABLE IF EXISTS metrics;
DROP TABLE IF EXISTS machines;
DROP TABLE IF EXISTS datasets;
DROP TABLE IF EXISTS applications;
DROP TABLE IF EXISTS datacollections;

--
-- Tables to maintain a data collection
--
CREATE TABLE datacollections(
    ID INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    description TEXT,
    created TEXT
);

CREATE TABLE applications(
    ID INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    description TEXT
);

CREATE TABLE datasets(
    ID INTEGER PRIMARY KEY,
    applicationID INTEGER REFERENCES applications(ID) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    name TEXT UNIQUE,
    description TEXT,
    created TEXT,
    url TEXT
);

CREATE TABLE machines(
    ID INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    description TEXT
);

CREATE TABLE metric_types(
    type TEXT PRIMARY KEY,
    seq INTEGER
);

INSERT INTO metric_types(type, seq) VALUES ('deterministic', 1);
INSERT INTO metric_types(type, seq) VALUES ('nondeterministic', 2);
INSERT INTO metric_types(type, seq) VALUES ('machine', 3);

CREATE TABLE metrics(
    ID INTEGER PRIMARY KEY,
    type TEXT REFERENCES metric_types(type),
    name TEXT UNIQUE,
    description TEXT
);

CREATE TABLE machine_metrics(
    machineID INTEGER REFERENCES machines(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metricID INTEGER REFERENCES metrics(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metric REAL
);

CREATE TABLE trials(
    ID INTEGER PRIMARY KEY,
    dataCollectionID INTEGER REFERENCES datacollections(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    machineID INTEGER REFERENCES machines(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    applicationID INTEGER REFERENCES applications(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    datasetID INTEGER REFERENCES datasets(ID)
        ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE INDEX trial_dset_idx ON trials(datasetID);

-- Maps nondeterministic characteristics onto a trial.
CREATE TABLE nondeterministic_metrics(
    trialID INTEGER REFERENCES trials(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metricID INTEGER REFERENCES metrics(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metric REAL
);

CREATE INDEX ndet_metrics_trial_idx ON nondeterministic_metrics(trialID);

-- Maps deterministic characteristics onto a dataset. 
CREATE TABLE deterministic_metrics(
    datasetID INTEGER REFERENCES datasets(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metricID INTEGER REFERENCES metrics(ID)
        ON DELETE CASCADE ON UPDATE CASCADE,
    metric REAL
);

CREATE INDEX det_metrics_dset_idx ON deterministic_metrics(datasetID);

