var express = require('express');
var HTTPStatus = require('http-status');
var crypto = require("crypto");
var db = require("../db");
var NewError = require("../error");
var base64 = require("../public/javascripts/base64");
var router = express.Router();


router.get("/", function(req, res, next) {
	// 신규 세션 발급
	if (req.session) {
		res.status(HTTPStatus.BAD_REQUEST).send({
			message: "로그아웃 후 요청하세요."
		});
		return;
	}
	
	var authHeader = req.get("Authorization");
	if (authHeader)
		authHeader = JSON.parse(authHeader);
		
	var invalidRequest = authHeader == null
					  || authHeader.id == null
					  || authHeader.password == null;
	if (invalidRequest) {
		// 정보 부족
		res.status(HTTPStatus.BAD_REQUEST).send({
			message: "인증 정보가 부족합니다."
		});
		return;
	}
	
	db.signIn(base64.decode(authHeader.id),
			  crypto.createHash("sha1").update(base64.decode(authHeader.password)).digest("hex"),
			  function(err, id, userinfo) {
				  if (err) {
					  // DB error
					  console.error(err);
					  next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
				  }
				  else if (userinfo) {
					  res.registerSession(userinfo, function(err, success) {
						  if (success) {
							  // 성공
							  res.send(); 
							  return;
						  }
						  // redis error 또는 세션키 발급 실패
						  if (err)
							  console.error(err);
						  next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
					  });
				  }
				  else {
					  // 인증 실패
					  res.status(HTTPStatus.BAD_REQUEST).send({
						  message: "ID나 Password가 일치하지 않습니다."
					  });
				  }
			  });
});


router.post("/", function(req, res, next) {
	// 회원 가입
	if (req.session) {
		res.status(HTTPStatus.BAD_REQUEST).send({
			message: "로그아웃 후 요청하세요."
		});
		return;
	}
	
	var info = req.body;
	if (info == null || info.id == null || info.password == null) {
		res.status(HTTPStatus.BAD_REQUEST).send({
			message: "가입을 위한 정보가 부족합니다."
		});
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
				// DB error
				console.error(err);
				next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			}
		}
		else {
			// 성공
			res.send();
		}
	});
});


router.put("/", function(req, res, next) {
	// 세션의 인증토큰 갱신
	if (req.session == null) {
		res.status(HTTPStatus.UNAUTHORIZED).send({
			message: "세션이 만료되었습니다."
		});
		return;
	}

	res.reissueToken(req.session, function(err, success) {
		if (err) {
			// redis error
			console.error(err);
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
		}
		else if (success) {
			// 성공
			res.send();
		}
		else {
			// 해킹시도 감지
			res.status(HTTPStatus.UNAUTHORIZED).send({
				message: "재인증이 필요합니다."
			});
		}
	});
});


router.delete("/", function(req, res, next) {
	// 세션 종료
	if (req.session)
		res.unregisterSession(req.session.key);
	res.send();
});

module.exports = router;
