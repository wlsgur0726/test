var express = require('express');
var HTTPStatus = require('http-status');
var db = require("../db");
var NewError = require("../error");
var router = express.Router();
var crypto = require("crypto");
var base64 = require("../public/javascripts/base64");

router.get("/", function(req, res, next) {
	if (req.session) {
		res.unregisterSession(req.session.key);
		res.session = null;
	}
	
	var authHeader = req.get("Authorization");
	if (authHeader)
		authHeader = JSON.parse(authHeader);
		
	var invalidRequest = authHeader == null
					  || authHeader.id == null
					  || authHeader.password == null;
	if (invalidRequest) {
		next(NewError(HTTPStatus.BAD_REQUEST));
		return;
	}
	
	db.signIn(base64.decode(authHeader.id),
			  crypto.createHash("sha1").update(base64.decode(authHeader.password)).digest("hex"),
			  function(err, id, userinfo) {
				  if (err) {
					  next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
				  }
				  else if (userinfo) {
					  res.registerSession(userinfo);
					  res.send();
				  }
				  else {
					  res.status(HTTPStatus.BAD_REQUEST).send({
						  message: "ID나 Password가 일치하지 않습니다."
					  });
				  }
			  });
});

router.post("/", function(req, res, next) {
	var info = req.body;
	if (info == null || info.id == null || info.password == null) {
		next(NewError(HTTPStatus.BAD_REQUEST));
		return;
	}
	var pw = crypto.createHash("sha1").update(info.password).digest("hex");
	db.signUp(info.id, pw, function(err, id, success) {
		if (err) {
			if (err.state == "23000") {
				res.status(HTTPStatus.BAD_REQUEST).send({
					message: "이미 존재하는 ID입니다."
				});
			}
			else {
				next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			}
		}
		else {
			res.send();
		}
	});
});

router.delete("/", function(req, res, next) {
	if (req.session == null) {
		next(NewError(HTTPStatus.BAD_REQUEST));
		return;
	}
	res.unregisterSession(req.session.key);
	res.send();
});

module.exports = router;
