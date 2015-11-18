/****** DB »ý¼º ******/
CREATE DATABASE [weboard]
GO

USE [weboard]
GO


/****** Object:  Table [dbo].[tb_user]    Script Date: 2015-11-19 06:54:29 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tb_user](
	[id] [nvarchar](50) NOT NULL,
	[password] [nvarchar](50) NOT NULL,
	[joindate] [datetime] NOT NULL CONSTRAINT [DF_user_joindate]  DEFAULT (getdate()),
 CONSTRAINT [PK_user] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO


/****** Object:  Table [dbo].[tb_board]    Script Date: 2015-11-19 06:55:09 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tb_board](
	[number] [int] IDENTITY(1,1) NOT NULL,
	[title] [nvarchar](100) NOT NULL,
	[writer] [nvarchar](50) NOT NULL,
	[register] [datetime] NOT NULL CONSTRAINT [DF_board_register]  DEFAULT (getdate()),
	[lastedit] [datetime] NOT NULL CONSTRAINT [DF_board_lastedit]  DEFAULT (getdate()),
	[content] [text] NOT NULL,
 CONSTRAINT [PK_board] PRIMARY KEY CLUSTERED 
(
	[number] DESC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO

ALTER TABLE [dbo].[tb_board]  WITH CHECK ADD  CONSTRAINT [FK_board_user] FOREIGN KEY([writer])
REFERENCES [dbo].[tb_user] ([id])
ON UPDATE CASCADE
ON DELETE CASCADE
GO

ALTER TABLE [dbo].[tb_board] CHECK CONSTRAINT [FK_board_user]
GO


/****** Object:  Table [dbo].[tb_comment]    Script Date: 2015-11-19 06:55:55 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tb_comment](
	[number] [int] IDENTITY(1,1) NOT NULL,
	[owner] [int] NOT NULL,
	[writer] [nvarchar](50) NOT NULL,
	[register] [datetime] NOT NULL CONSTRAINT [DF_comment_register]  DEFAULT (getdate()),
	[comment] [nvarchar](200) NOT NULL,
 CONSTRAINT [PK_comment] PRIMARY KEY CLUSTERED 
(
	[number] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO

ALTER TABLE [dbo].[tb_comment]  WITH CHECK ADD  CONSTRAINT [FK_comment_user] FOREIGN KEY([writer])
REFERENCES [dbo].[tb_user] ([id])
ON UPDATE CASCADE
ON DELETE CASCADE
GO

ALTER TABLE [dbo].[tb_comment] CHECK CONSTRAINT [FK_comment_user]
GO


/****** Object:  StoredProcedure [dbo].[sp_addContent]    Script Date: 2015-11-19 06:57:34 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

ALTER PROCEDURE [dbo].[sp_addContent]
	@aTitle NVARCHAR(100),
	@aWriter NVARCHAR(50),
	@aContent TEXT
AS
BEGIN
	insert into tb_board(title, writer, content) values (@aTitle, @aWriter, @aContent);
	SELECT @@IDENTITY as number;
END

GO


/****** Object:  StoredProcedure [dbo].[sp_addComment]    Script Date: 2015-11-19 06:58:19 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[sp_addComment]
	@aOwner INT, 
	@aWriter NVARCHAR(50),
	@aComment NVARCHAR(200)
AS
BEGIN
    insert into tb_comment(owner, writer, comment) values (@aOwner, @aWriter, @aComment);
	SELECT @@IDENTITY as number;
END

GO


/****** Object:  StoredProcedure [dbo].[sp_getContentList]    Script Date: 2015-11-19 06:58:51 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[sp_getContentList]
	@aStart INT,
	@aCount INT
AS
BEGIN
	SET NOCOUNT ON;

	with temptable as (
		select row_number() over(order by number) as rownum, number, title, writer, register, lastedit
		from tb_board
	)

	select number, title, writer, register, lastedit
	from tb_temptable
	where rownum > @aStart and rownum <= (@aStart+@aCount);
END

GO


/****** Object:  StoredProcedure [dbo].[sp_getContent]    Script Date: 2015-11-19 06:59:23 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[sp_getContent]
	@aNumber INT
AS
BEGIN
	SET NOCOUNT ON;
	select content from tb_board where number=@aNumber;
    select * from tb_comment where owner=@aNumber;
END

GO


