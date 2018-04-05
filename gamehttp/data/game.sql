/*
Navicat MySQL Data Transfer

Source Server         : 120.27.6.212-mysql
Source Server Version : 50721
Source Host           : 120.27.6.212:3306
Source Database       : game

Target Server Type    : MYSQL
Target Server Version : 50721
File Encoding         : 65001

Date: 2018-04-05 16:58:39
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for gameuser
-- ----------------------------
DROP TABLE IF EXISTS `gameuser`;
CREATE TABLE `gameuser` (
  `uid` int(11) NOT NULL,
  `username` varchar(255) DEFAULT NULL,
  `userpasswd` varchar(255) DEFAULT NULL,
  `nickname` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `gameuser_uid_unique_index` (`uid`) USING BTREE,
  UNIQUE KEY `gameuser_username_unique_index` (`username`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
