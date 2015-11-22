var express = require('express');
var HTTPStatus = require('http-status');
var db = require("../db");
var NewError = require("../error");
var router = express.Router();
var crypto = require('crypto');
var base64 = require('../base64');

router.get("/", function(req, res, next) {
	if (req.session != null) {
		// delete...
		return;
	}
	
	var authHeader = base64.decode(req.get("Authorization"));
	console.log("authHeader : " + authHeader);
	var info = JSON.parse(authHeader);
	
	var pw = crypto.createHash('sha1').update(info.password).digest('hex');
	db.signIn(info.id, pw, function(err, id, userinfo) {
		if (err) {
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
		}
		else if (userinfo == null) {
			res.send({
				success: false,
				reason: "인증 실패"
			});
		}
		else {
			res.registerSession(userinfo);
			res.send({success: true});
		}
	});
});

router.post("/", function(req, res, next) {
	var info = req.body;
	if (info.id == null || info.password == null) {
		next(NewError(HTTPStatus.BAD_REQUEST));
		return;
	}
	
	var pw = crypto.createHash('sha1').update(info.password).digest('hex');
	db.signUp(info.id, pw, function(err, id, success) {
		res.send({success: success});
	});
});

module.exports = router;
