var express = require('express');
var HTTPStatus = require('http-status');
var db = require("../db");
var NewError = require("../error");
var router = express.Router();


function is_not_a_number(p) {
	return p==null ? false : isNaN(p);
}


router.get("/", function(req, res, next) {
	// 글 목록 조회
	var start = req.query.start;
	var count = req.query.count;
	
	if (is_not_a_number(start) || is_not_a_number(count)) {
		res.status(HTTPStatus.BAD_REQUEST).send({
			message: "잘못된 파라미터입니다."
		});
		return;
	}
	
	if (start == null || start < 0)
		start = 0;
	if (count == null || count < 1)
		count = 10;
	
	db.getContentList(start, count, function(err, contents) {
		if (err) {
			console.error(err);
			next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
			return;
		}
		res.send(contents);
	});
});


module.exports = router;
