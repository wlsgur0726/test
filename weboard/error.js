var HTTPStatus = require('http-status');

module.exports = function(status_code, info) {
	var e = new Error(HTTPStatus[status_code]);
	e.status = status_code;
	e.info = info;
	return e;
};
