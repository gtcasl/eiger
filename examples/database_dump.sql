-- MySQL dump 10.13  Distrib 5.5.24, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version	5.5.24-0ubuntu0.12.04.1

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
INSERT INTO `datacollections` VALUES (1,'test','test dc','2012-09-21 15:49:08');
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
INSERT INTO `datasets` VALUES (1,1,'test','test dataset','2012-09-21 15:49:08','test dataset url');
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
  `metric` double DEFAULT NULL
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
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=101 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `executions`
--

LOCK TABLES `executions` WRITE;
/*!40000 ALTER TABLE `executions` DISABLE KEYS */;
INSERT INTO `executions` VALUES (1,1,1),(2,1,2),(3,1,3),(4,1,4),(5,1,5),(6,1,6),(7,1,7),(8,1,8),(9,1,9),(10,1,10),(11,1,11),(12,1,12),(13,1,13),(14,1,14),(15,1,15),(16,1,16),(17,1,17),(18,1,18),(19,1,19),(20,1,20),(21,1,21),(22,1,22),(23,1,23),(24,1,24),(25,1,25),(26,1,26),(27,1,27),(28,1,28),(29,1,29),(30,1,30),(31,1,31),(32,1,32),(33,1,33),(34,1,34),(35,1,35),(36,1,36),(37,1,37),(38,1,38),(39,1,39),(40,1,40),(41,1,41),(42,1,42),(43,1,43),(44,1,44),(45,1,45),(46,1,46),(47,1,47),(48,1,48),(49,1,49),(50,1,50),(51,1,51),(52,1,52),(53,1,53),(54,1,54),(55,1,55),(56,1,56),(57,1,57),(58,1,58),(59,1,59),(60,1,60),(61,1,61),(62,1,62),(63,1,63),(64,1,64),(65,1,65),(66,1,66),(67,1,67),(68,1,68),(69,1,69),(70,1,70),(71,1,71),(72,1,72),(73,1,73),(74,1,74),(75,1,75),(76,1,76),(77,1,77),(78,1,78),(79,1,79),(80,1,80),(81,1,81),(82,1,82),(83,1,83),(84,1,84),(85,1,85),(86,1,86),(87,1,87),(88,1,88),(89,1,89),(90,1,90),(91,1,91),(92,1,92),(93,1,93),(94,1,94),(95,1,95),(96,1,96),(97,1,97),(98,1,98),(99,1,99),(100,1,100);
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
INSERT INTO `nondeterministic_metrics` VALUES (1,1,10),(1,2,0.000013448),(2,1,20),(2,2,0.000091322),(3,1,30),(3,2,0.000295513),(4,1,40),(4,2,0.000657939),(5,1,50),(5,2,0.00128571),(6,1,60),(6,2,0.00224521),(7,1,70),(7,2,0.0037365),(8,1,80),(8,2,0.00558392),(9,1,90),(9,2,0.00802058),(10,1,100),(10,2,0.0109095),(11,1,110),(11,2,0.0125438),(12,1,120),(12,2,0.0122987),(13,1,130),(13,2,0.0179143),(14,1,140),(14,2,0.0200159),(15,1,150),(15,2,0.0218574),(16,1,160),(16,2,0.0279774),(17,1,170),(17,2,0.0327839),(18,1,180),(18,2,0.0416128),(19,1,190),(19,2,0.0460382),(20,1,200),(20,2,0.0520169),(21,1,210),(21,2,0.0622155),(22,1,220),(22,2,0.0659929),(23,1,230),(23,2,0.0745064),(24,1,240),(24,2,0.0851219),(25,1,250),(25,2,0.0982503),(26,1,260),(26,2,0.108733),(27,1,270),(27,2,0.122992),(28,1,280),(28,2,0.136681),(29,1,290),(29,2,0.151481),(30,1,300),(30,2,0.17231),(31,1,310),(31,2,0.184877),(32,1,320),(32,2,0.203944),(33,1,330),(33,2,0.219729),(34,1,340),(34,2,0.249693),(35,1,350),(35,2,0.273232),(36,1,360),(36,2,0.297578),(37,1,370),(37,2,0.338265),(38,1,380),(38,2,0.378033),(39,1,390),(39,2,0.410221),(40,1,400),(40,2,0.450528),(41,1,410),(41,2,0.49435),(42,1,420),(42,2,0.551054),(43,1,430),(43,2,0.602314),(44,1,440),(44,2,0.642782),(45,1,450),(45,2,0.697415),(46,1,460),(46,2,0.748532),(47,1,470),(47,2,0.799673),(48,1,480),(48,2,0.857278),(49,1,490),(49,2,0.91619),(50,1,500),(50,2,0.973278),(51,1,510),(51,2,1.03514),(52,1,520),(52,2,1.11028),(53,1,530),(53,2,1.16648),(54,1,540),(54,2,1.23527),(55,1,550),(55,2,1.31823),(56,1,560),(56,2,1.38056),(57,1,570),(57,2,1.46197),(58,1,580),(58,2,1.54177),(59,1,590),(59,2,1.61986),(60,1,600),(60,2,1.70531),(61,1,610),(61,2,1.81816),(62,1,620),(62,2,1.88729),(63,1,630),(63,2,2.00028),(64,1,640),(64,2,2.07918),(65,1,650),(65,2,2.17941),(66,1,660),(66,2,2.28354),(67,1,670),(67,2,2.41139),(68,1,680),(68,2,2.52054),(69,1,690),(69,2,2.60812),(70,1,700),(70,2,2.73368),(71,1,710),(71,2,2.88218),(72,1,720),(72,2,2.97481),(73,1,730),(73,2,3.10777),(74,1,740),(74,2,3.25702),(75,1,750),(75,2,3.38923),(76,1,760),(76,2,3.53477),(77,1,770),(77,2,3.68598),(78,1,780),(78,2,3.88786),(79,1,790),(79,2,4.05423),(80,1,800),(80,2,4.21947),(81,1,810),(81,2,4.35341),(82,1,820),(82,2,4.55175),(83,1,830),(83,2,4.70659),(84,1,840),(84,2,4.9552),(85,1,850),(85,2,5.09206),(86,1,860),(86,2,5.29392),(87,1,870),(87,2,5.56805),(88,1,880),(88,2,5.77291),(89,1,890),(89,2,5.95458),(90,1,900),(90,2,6.22808),(91,1,910),(91,2,6.40309),(92,1,920),(92,2,6.68221),(93,1,930),(93,2,6.81926),(94,1,940),(94,2,7.04442),(95,1,950),(95,2,7.3691),(96,1,960),(96,2,7.60147),(97,1,970),(97,2,7.86852),(98,1,980),(98,2,8.01928),(99,1,990),(99,2,8.24926),(100,1,1000),(100,2,8.65286);
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
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=101 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `trials`
--

LOCK TABLES `trials` WRITE;
/*!40000 ALTER TABLE `trials` DISABLE KEYS */;
INSERT INTO `trials` VALUES (1,1,1,1,1,0),(2,1,1,1,1,0),(3,1,1,1,1,0),(4,1,1,1,1,0),(5,1,1,1,1,0),(6,1,1,1,1,0),(7,1,1,1,1,0),(8,1,1,1,1,0),(9,1,1,1,1,0),(10,1,1,1,1,0),(11,1,1,1,1,0),(12,1,1,1,1,0),(13,1,1,1,1,0),(14,1,1,1,1,0),(15,1,1,1,1,0),(16,1,1,1,1,0),(17,1,1,1,1,0),(18,1,1,1,1,0),(19,1,1,1,1,0),(20,1,1,1,1,0),(21,1,1,1,1,0),(22,1,1,1,1,0),(23,1,1,1,1,0),(24,1,1,1,1,0),(25,1,1,1,1,0),(26,1,1,1,1,0),(27,1,1,1,1,0),(28,1,1,1,1,0),(29,1,1,1,1,0),(30,1,1,1,1,0),(31,1,1,1,1,0),(32,1,1,1,1,0),(33,1,1,1,1,0),(34,1,1,1,1,0),(35,1,1,1,1,0),(36,1,1,1,1,0),(37,1,1,1,1,0),(38,1,1,1,1,0),(39,1,1,1,1,0),(40,1,1,1,1,0),(41,1,1,1,1,0),(42,1,1,1,1,0),(43,1,1,1,1,0),(44,1,1,1,1,0),(45,1,1,1,1,0),(46,1,1,1,1,0),(47,1,1,1,1,0),(48,1,1,1,1,0),(49,1,1,1,1,0),(50,1,1,1,1,0),(51,1,1,1,1,0),(52,1,1,1,1,0),(53,1,1,1,1,0),(54,1,1,1,1,0),(55,1,1,1,1,0),(56,1,1,1,1,0),(57,1,1,1,1,0),(58,1,1,1,1,0),(59,1,1,1,1,0),(60,1,1,1,1,0),(61,1,1,1,1,0),(62,1,1,1,1,0),(63,1,1,1,1,0),(64,1,1,1,1,0),(65,1,1,1,1,0),(66,1,1,1,1,0),(67,1,1,1,1,0),(68,1,1,1,1,0),(69,1,1,1,1,0),(70,1,1,1,1,0),(71,1,1,1,1,0),(72,1,1,1,1,0),(73,1,1,1,1,0),(74,1,1,1,1,0),(75,1,1,1,1,0),(76,1,1,1,1,0),(77,1,1,1,1,0),(78,1,1,1,1,0),(79,1,1,1,1,0),(80,1,1,1,1,0),(81,1,1,1,1,0),(82,1,1,1,1,0),(83,1,1,1,1,0),(84,1,1,1,1,0),(85,1,1,1,1,0),(86,1,1,1,1,0),(87,1,1,1,1,0),(88,1,1,1,1,0),(89,1,1,1,1,0),(90,1,1,1,1,0),(91,1,1,1,1,0),(92,1,1,1,1,0),(93,1,1,1,1,0),(94,1,1,1,1,0),(95,1,1,1,1,0),(96,1,1,1,1,0),(97,1,1,1,1,0),(98,1,1,1,1,0),(99,1,1,1,1,0),(100,1,1,1,1,0);
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

-- Dump completed on 2012-09-21 15:59:35
