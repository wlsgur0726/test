var express = require('express');
var HTTPStatus = require('http-status');
var db = require("../db");
var NewError = require("../error");
var router = express.Router();

function validCheck_session(req, res) {
	if (req.session == null) {
		res.status(HTTPStatus.UNAUTHORIZED).send({
			message: "권한이 없습니다."
		});
		return false;
	}
	return true;
}

function validCheck_param(req, res, paramNames) {
	var ret = [];
	for (var i=0; i<paramNames.length; ++i) {	
		var num = req.params[paramNames[i]];
		if (isNaN(num)) {
			res.status(HTTPStatus.BAD_REQUEST).send({
				message: "잘못된 파라미터입니다."
			});
			return null;
		}
		ret[i] = num;
	}
	return ret.length == 0 ? null : ret;
}

function validCheck_all(req, res, paramNames) {
	if (validCheck_session(req, res))
		return validCheck_param(req, res, paramNames);
	return null;
}

router.get("/:num", function(req, res, next) {
	var num = validCheck_param(req, res, ["num"]);
	if (num == null)
		return;

	db.getContent(num[0], function(err, content, comments) {
		if (err) {
			console.error(err);
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			return;
		}
		res.send({content:content, comments:comments});
	});
});

router.post("/", function(req, res, next) {
	if (validCheck_session(req, res) == false)
		return;

	db.addContent(req.body.title,
				  req.session.id,
				  req.body.content,
				  function(err, number){
					  if (err) {
						  console.error(err);
						  next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
						  return;
					  }
					  console.log("post success " + number);
					  res.send({number: number});
				  });
});

router.post("/:num/comment", function(req, res, next) {
	var num = validCheck_all(req, res, ["num"]);
	if (num == null)
		return;

	db.addComment(num[0],
				  req.session.id,
				  req.body.comment,
				  function(err, number){
					  if (err) {
						  console.error(err);
						  next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
						  return;
					  }
					  res.send({number: number});
				  });
});

router.put("/:num", function(req, res, next) {
	var num = validCheck_all(req, res, ["num"]);
	if (num == null)
		return;

	db.editContent(num[0],
				   req.body.title,
				   req.session.id,
				   req.body.content,
				   function(err, number){
					   if (err) {
						   console.error(err);
						   next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
						   return;
					   }
					   res.send({number: number});
				   });
});

router.delete("/:num", function(req, res, next) {
	var num = validCheck_all(req, res, ["num"]);
	if (num == null)
		return;

	db.deleteContent(num[0], req.session.id, function(err, number) {
		if (err) {
			console.error(err);
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			return;
		}
		res.send({number: number});
	});
});

router.delete("/:contentNum/comment/:commentNum", function(req, res, next) {
	var nums = validCheck_all(req, res, ["contentNum", "commentNum"]);
	if (nums == null)
		return;

	db.deleteComment(nums[1],
					 req.session.id,
					 function(err, number){
						 if (err) {
							 console.error(err);
							 next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
							 return;
						 }
						 res.send({number: number});
					 });
});

module.exports = router;
