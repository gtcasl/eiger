-- MySQL dump 10.13  Distrib 5.5.34, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version	5.5.34-0ubuntu0.13.10.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `applications`
--

DROP TABLE IF EXISTS `applications`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `applications` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(256) DEFAULT NULL,
  `description` text,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `applications`
--

LOCK TABLES `applications` WRITE;
/*!40000 ALTER TABLE `applications` DISABLE KEYS */;
INSERT INTO `applications` VALUES (1,'test','test application');
/*!40000 ALTER TABLE `applications` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `datacollections`
--

DROP TABLE IF EXISTS `datacollections`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `datacollections` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(256) DEFAULT NULL,
  `description` text,
  `created` datetime DEFAULT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `datacollections`
--

LOCK TABLES `datacollections` WRITE;
/*!40000 ALTER TABLE `datacollections` DISABLE KEYS */;
INSERT INTO `datacollections` VALUES (1,'example','test dc','2013-11-04 16:13:00');
/*!40000 ALTER TABLE `datacollections` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `datasets`
--

DROP TABLE IF EXISTS `datasets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `datasets` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `applicationID` int(11) DEFAULT NULL,
  `name` varchar(256) DEFAULT NULL,
  `description` text,
  `created` datetime DEFAULT NULL,
  `url` text,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `datasets`
--

LOCK TABLES `datasets` WRITE;
/*!40000 ALTER TABLE `datasets` DISABLE KEYS */;
INSERT INTO `datasets` VALUES (1,1,'test','test dataset','2013-11-04 16:13:00','test dataset url');
/*!40000 ALTER TABLE `datasets` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `deterministic_metrics`
--

DROP TABLE IF EXISTS `deterministic_metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `deterministic_metrics` (
  `datasetID` int(11) DEFAULT NULL,
  `metricID` int(11) DEFAULT NULL,
  `metric` double DEFAULT NULL,
  KEY `datasetID` (`datasetID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `deterministic_metrics`
--

LOCK TABLES `deterministic_metrics` WRITE;
/*!40000 ALTER TABLE `deterministic_metrics` DISABLE KEYS */;
/*!40000 ALTER TABLE `deterministic_metrics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `executions`
--

DROP TABLE IF EXISTS `executions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `executions` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `machineID` int(11) DEFAULT NULL,
  `trialID` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `trialID` (`trialID`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `executions`
--

LOCK TABLES `executions` WRITE;
/*!40000 ALTER TABLE `executions` DISABLE KEYS */;
INSERT INTO `executions` VALUES (1,1,1),(2,1,2),(3,1,3),(4,1,4),(5,1,5),(6,1,6),(7,1,7),(8,1,8),(9,1,9),(10,1,10),(11,1,11),(12,1,12),(13,1,13),(14,1,14),(15,1,15),(16,1,16),(17,1,17),(18,1,18),(19,1,19),(20,1,20),(21,1,21),(22,1,22),(23,1,23),(24,1,24),(25,1,25),(26,1,26),(27,1,27),(28,1,28),(29,1,29),(30,1,30),(31,1,31),(32,1,32),(33,1,33),(34,1,34),(35,1,35),(36,1,36),(37,1,37),(38,1,38),(39,1,39),(40,1,40),(41,1,41),(42,1,42),(43,1,43),(44,1,44),(45,1,45),(46,1,46),(47,1,47),(48,1,48),(49,1,49),(50,1,50);
/*!40000 ALTER TABLE `executions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `machine_metrics`
--

DROP TABLE IF EXISTS `machine_metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `machine_metrics` (
  `machineID` int(11) DEFAULT NULL,
  `metricID` int(11) DEFAULT NULL,
  `metric` double DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `machine_metrics`
--

LOCK TABLES `machine_metrics` WRITE;
/*!40000 ALTER TABLE `machine_metrics` DISABLE KEYS */;
/*!40000 ALTER TABLE `machine_metrics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `machines`
--

DROP TABLE IF EXISTS `machines`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `machines` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(256) DEFAULT NULL,
  `description` text,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `machines`
--

LOCK TABLES `machines` WRITE;
/*!40000 ALTER TABLE `machines` DISABLE KEYS */;
INSERT INTO `machines` VALUES (1,'test','test machine');
/*!40000 ALTER TABLE `machines` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `matrices`
--

DROP TABLE IF EXISTS `matrices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `matrices` (
  `ID` int(11) DEFAULT NULL,
  `matrix` longtext
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `matrices`
--

LOCK TABLES `matrices` WRITE;
/*!40000 ALTER TABLE `matrices` DISABLE KEYS */;
/*!40000 ALTER TABLE `matrices` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `metrics`
--

DROP TABLE IF EXISTS `metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `metrics` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `type` enum('result','deterministic','nondeterministic','machine','other') DEFAULT NULL,
  `name` varchar(256) DEFAULT NULL,
  `description` text,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `metrics`
--

LOCK TABLES `metrics` WRITE;
/*!40000 ALTER TABLE `metrics` DISABLE KEYS */;
INSERT INTO `metrics` VALUES (1,'nondeterministic','size','matrix size'),(2,'result','runtime','runtime of this matmul');
/*!40000 ALTER TABLE `metrics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `model_functions`
--

DROP TABLE IF EXISTS `model_functions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `model_functions` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `function` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `function` (`function`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `model_functions`
--

LOCK TABLES `model_functions` WRITE;
/*!40000 ALTER TABLE `model_functions` DISABLE KEYS */;
/*!40000 ALTER TABLE `model_functions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `model_map`
--

DROP TABLE IF EXISTS `model_map`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `model_map` (
  `predictorID` int(11) DEFAULT NULL,
  `functionID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `model_map`
--

LOCK TABLES `model_map` WRITE;
/*!40000 ALTER TABLE `model_map` DISABLE KEYS */;
/*!40000 ALTER TABLE `model_map` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `model_predictors`
--

DROP TABLE IF EXISTS `model_predictors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `model_predictors` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `modelID` int(11) DEFAULT NULL,
  `beta` double DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `model_predictors`
--

LOCK TABLES `model_predictors` WRITE;
/*!40000 ALTER TABLE `model_predictors` DISABLE KEYS */;
/*!40000 ALTER TABLE `model_predictors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `models`
--

DROP TABLE IF EXISTS `models`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `models` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `dataCollectionID` int(11) DEFAULT NULL,
  `trainingPCs` int(11) DEFAULT NULL,
  `machinePCs` int(11) DEFAULT NULL,
  `regression` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `models`
--

LOCK TABLES `models` WRITE;
/*!40000 ALTER TABLE `models` DISABLE KEYS */;
/*!40000 ALTER TABLE `models` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `nondeterministic_metrics`
--

DROP TABLE IF EXISTS `nondeterministic_metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `nondeterministic_metrics` (
  `executionID` int(11) DEFAULT NULL,
  `metricID` int(11) DEFAULT NULL,
  `metric` double DEFAULT NULL,
  KEY `executionID` (`executionID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `nondeterministic_metrics`
--

LOCK TABLES `nondeterministic_metrics` WRITE;
/*!40000 ALTER TABLE `nondeterministic_metrics` DISABLE KEYS */;
INSERT INTO `nondeterministic_metrics` VALUES (1,1,10),(1,2,0.000013574000149674248),(2,1,20),(2,2,0.00008635300036985427),(3,1,30),(3,2,0.00028152900631539524),(4,1,40),(4,2,0.0006578390020877123),(5,1,50),(5,2,0.0013129579601809382),(6,1,60),(6,2,0.0022424899507313967),(7,1,70),(7,2,0.0037216059863567352),(8,1,80),(8,2,0.005568691063672304),(9,1,90),(9,2,0.007976376451551914),(10,1,100),(10,2,0.010953961871564388),(11,1,110),(11,2,0.010005735792219639),(12,1,120),(12,2,0.017818816006183624),(13,1,130),(13,2,0.01683766022324562),(14,1,140),(14,2,0.021886315196752548),(15,1,150),(15,2,0.02213934436440468),(16,1,160),(16,2,0.02802852727472782),(17,1,170),(17,2,0.031203659251332283),(18,1,180),(18,2,0.03939375281333923),(19,1,190),(19,2,0.04518747702240944),(20,1,200),(20,2,0.05332903191447258),(21,1,210),(21,2,0.05883931741118431),(22,1,220),(22,2,0.06861627846956253),(23,1,230),(23,2,0.07587053626775742),(24,1,240),(24,2,0.08834800124168396),(25,1,250),(25,2,0.09826383739709854),(26,1,260),(26,2,0.11330963671207428),(27,1,270),(27,2,0.12578493356704712),(28,1,280),(28,2,0.13691174983978271),(29,1,290),(29,2,0.14917433261871338),(30,1,300),(30,2,0.16465799510478973),(31,1,310),(31,2,0.18293048441410065),(32,1,320),(32,2,0.19930754601955414),(33,1,330),(33,2,0.21901795268058777),(34,1,340),(34,2,0.2389945536851883),(35,1,350),(35,2,0.2601310610771179),(36,1,360),(36,2,0.28227299451828003),(37,1,370),(37,2,0.31300872564315796),(38,1,380),(38,2,0.34275197982788086),(39,1,390),(39,2,0.3662860691547394),(40,1,400),(40,2,0.3974011242389679),(41,1,410),(41,2,0.42652666568756104),(42,1,420),(42,2,0.4678702652454376),(43,1,430),(43,2,0.49566107988357544),(44,1,440),(44,2,0.538007915019989),(45,1,450),(45,2,0.5899955034255981),(46,1,460),(46,2,0.6357541084289551),(47,1,470),(47,2,0.6758474707603455),(48,1,480),(48,2,0.7263821363449097),(49,1,490),(49,2,0.7822171449661255),(50,1,500),(50,2,0.8283967971801758);
/*!40000 ALTER TABLE `nondeterministic_metrics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `principal_component_analyses`
--

DROP TABLE IF EXISTS `principal_component_analyses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `principal_component_analyses` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `dataCollectionID` int(11) DEFAULT NULL,
  `machinePC` longtext,
  `machineWeight` longtext,
  `trainingPC` longtext,
  `trainingWeight` longtext,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `principal_component_analyses`
--

LOCK TABLES `principal_component_analyses` WRITE;
/*!40000 ALTER TABLE `principal_component_analyses` DISABLE KEYS */;
/*!40000 ALTER TABLE `principal_component_analyses` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `properties`
--

DROP TABLE IF EXISTS `properties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `properties` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `trialID` int(11) DEFAULT NULL,
  `propertyName` text,
  `property` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `properties`
--

LOCK TABLES `properties` WRITE;
/*!40000 ALTER TABLE `properties` DISABLE KEYS */;
/*!40000 ALTER TABLE `properties` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `trials`
--

DROP TABLE IF EXISTS `trials`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `trials` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `dataCollectionID` int(11) DEFAULT NULL,
  `machineID` int(11) DEFAULT NULL,
  `applicationID` int(11) DEFAULT NULL,
  `datasetID` int(11) DEFAULT NULL,
  `propertiesID` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `datasetID` (`datasetID`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `trials`
--

LOCK TABLES `trials` WRITE;
/*!40000 ALTER TABLE `trials` DISABLE KEYS */;
INSERT INTO `trials` VALUES (1,1,1,1,1,0),(2,1,1,1,1,0),(3,1,1,1,1,0),(4,1,1,1,1,0),(5,1,1,1,1,0),(6,1,1,1,1,0),(7,1,1,1,1,0),(8,1,1,1,1,0),(9,1,1,1,1,0),(10,1,1,1,1,0),(11,1,1,1,1,0),(12,1,1,1,1,0),(13,1,1,1,1,0),(14,1,1,1,1,0),(15,1,1,1,1,0),(16,1,1,1,1,0),(17,1,1,1,1,0),(18,1,1,1,1,0),(19,1,1,1,1,0),(20,1,1,1,1,0),(21,1,1,1,1,0),(22,1,1,1,1,0),(23,1,1,1,1,0),(24,1,1,1,1,0),(25,1,1,1,1,0),(26,1,1,1,1,0),(27,1,1,1,1,0),(28,1,1,1,1,0),(29,1,1,1,1,0),(30,1,1,1,1,0),(31,1,1,1,1,0),(32,1,1,1,1,0),(33,1,1,1,1,0),(34,1,1,1,1,0),(35,1,1,1,1,0),(36,1,1,1,1,0),(37,1,1,1,1,0),(38,1,1,1,1,0),(39,1,1,1,1,0),(40,1,1,1,1,0),(41,1,1,1,1,0),(42,1,1,1,1,0),(43,1,1,1,1,0),(44,1,1,1,1,0),(45,1,1,1,1,0),(46,1,1,1,1,0),(47,1,1,1,1,0),(48,1,1,1,1,0),(49,1,1,1,1,0),(50,1,1,1,1,0);
/*!40000 ALTER TABLE `trials` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `vectors`
--

DROP TABLE IF EXISTS `vectors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vectors` (
  `ID` int(11) DEFAULT NULL,
  `vector` longtext
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `vectors`
--

LOCK TABLES `vectors` WRITE;
/*!40000 ALTER TABLE `vectors` DISABLE KEYS */;
/*!40000 ALTER TABLE `vectors` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-11-04 16:18:30
