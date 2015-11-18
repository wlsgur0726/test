CREATE DATABASE `weboard` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;
USE `weboard`;


CREATE TABLE `tb_user` (
  `id` varchar(45) COLLATE utf8_bin NOT NULL,
  `password` varchar(45) COLLATE utf8_bin NOT NULL,
  `joindate` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


CREATE TABLE `tb_board` (
  `number` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `writer` varchar(45) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `register` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `lastedit` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `content` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`number`),
  KEY `writer_idx` (`writer`),
  CONSTRAINT `board_writer` FOREIGN KEY (`writer`) REFERENCES `tb_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3601 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE `tb_comment` (
  `number` int(11) NOT NULL AUTO_INCREMENT,
  `owner` int(11) NOT NULL,
  `writer` varchar(45) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `register` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `comment` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`number`),
  KEY `comment_writer_idx` (`writer`),
  KEY `comment_owner_idx` (`owner`),
  CONSTRAINT `comment_owner` FOREIGN KEY (`owner`) REFERENCES `tb_board` (`number`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `comment_writer` FOREIGN KEY (`writer`) REFERENCES `tb_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5088 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


DELIMITER $$
CREATE DEFINER=`root`@`%` PROCEDURE `sp_addContent`(
	IN aTitle VARCHAR(100),
    IN aWriter VARCHAR(45),
    IN aContent TEXT
)
BEGIN
	insert into tb_board(title, writer, content) values (aTitle, aWriter, aContent);
    select last_insert_id() as number;
END$$
DELIMITER ;


DELIMITER $$
CREATE DEFINER=`root`@`%` PROCEDURE `sp_addComment`(
	IN aOwner INT,
    IN aWriter VARCHAR(45),
    IN aComment TEXT)
BEGIN
	insert into tb_comment(owner, writer, comment) values (aOwner, aWriter, aComment);
    select last_insert_id() as number;
END$$
DELIMITER ;


DELIMITER $$
CREATE DEFINER=`root`@`%` PROCEDURE `sp_getContentList`(
	IN aStart INT,
    IN aCount INT
)
BEGIN
	select number, title, writer, register, lastedit 
    from tb_board
    order by number desc
    limit aStart, aCount;
END$$
DELIMITER ;


DELIMITER $$
CREATE DEFINER=`root`@`%` PROCEDURE `sp_getContent`(
	IN aNumber INT
)
BEGIN
	select content from tb_board where number=aNumber;
    select * from tb_comment where owner=aNumber;
END$$
DELIMITER ;

