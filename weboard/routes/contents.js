var express = require('express');
var HTTPStatus = require('http-status');
var db = require("../db");
var NewError = require("../error");
var router = express.Router();

router.get("/:num", function(req, res, next) {
	var num = req.params.num;
	if (isNaN(num)) {
		next(NewError(HTTPStatus.BAD_REQUEST));
		return;
	}
	db.getContent(num, function(err, content, comments) {
		if (err) {
			console.error(err);
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			return;
		}
		res.send({content:content, comments:comments});
	});
});

router.post("/", function(req, res, next) {
	console.log("post");
	console.log(req.body);
	res.send("post");
	
	var content = req.body;
});

router.put("/:num", function(req, res, next) {
	var num = req.params.num;
	console.log("put " + num);
	console.log(req.body);
	res.send("put " + num);
});

router.delete("/:num", function(req, res, next) {
	var num = req.params.num;
	console.log("delete " + num);
	res.send("delete " + num);
});

module.exports = router;
