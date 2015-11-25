var Redis = require('redis');
var cookie = require('cookie');
var uuid = require('node-uuid');
var HashMap = require('hashmap');
var crypto = require('crypto');
var NewError = require("./error");

const HMAC_Key = "key";
const expireSec = 5 * 60;

function NewClient(pool) {	
	var client = Redis.createClient(pool.options);
	client.on("error", function (err) {
		console.error("[RedisError] " + err);
		var del = function(list) {
			var i = list.indexOf(client);
			if (i != -1) {
				list.splice(i, 1);
				return true;
			}
			return false;
		};
		if (del(pool.pool) == false)
			del(pool.stage);
		client.end();
	});
	return client;
}

function Pool(options) {
	const Default = {
		host: "127.0.0.1",
		port: 6379,
		min: 1,
		max: 30,
		init: true
	};
	
	if (options == null)
		options = Default;
	else {
		if (options.host == null)
			options.host = Default.host;
		if (options.port == null)
			options.port = Default.port;
		if (options.min == null)
			options.min = Default.min;
		if (options.max == null)
			options.max = Default.max;
		if (options.init == null)
			options.init = Default.init;
		
		if (options.min > options.max)
			options.min = options.max;
	}
	
	this.options = options;
	this.requests = new Array();
	this.stage = new Array();
	this.pool = new Array();
	for (var i=0; i<this.options.min; ++i) {
		this.pool.push(NewClient(this));
	}
}

Pool.prototype.next = function() {
	if (this.requests.length == 0)
		return;
	if (this.stage.length >= this.options.max)
		return;

	var redis;
	if (this.pool.length > 0)
		redis = this.pool.shift();
	else
		redis = NewClient(this);
	
	this.stage.push(redis);
	var req = this.requests.shift();
	req(redis);
};

Pool.prototype.open = function(callback) {
	this.requests.push(callback);
	this.next();
};

Pool.prototype.close = function(redis) {
	var i = this.stage.indexOf(redis);
	if (i != -1)
		this.stage.splice(i, 1);

	if (this.pool.length + this.stage.length < this.options.max) {
		if (this.options.init) {
			redis.quit();
			this.pool.push(NewClient(this));
		}
		else {
			this.pool.push(redis);
		}
	}
	else {
		redis.quit();
	}
	this.next();
};

Pool.prototype.closeAll = function() {
	var clear = function(list) {
		for (var i=0; i<list.length; ++i)
			list[i].end();
	};
	clear(this.stage);
	clear(this.pool);
};

var pool = new Pool({
	host: "192.168.88.130",
	port: 6379
});

function hash(data) {
	return crypto.createHmac("sha1", HMAC_Key).update(data).digest("base64");
}
function resetNonce(res, session) {
	session.nonce = Math.random();
	session.HMAC = hash(session.nonce.toString());
	res.cookie("session", JSON.stringify(session));
}

var candidateSessions = new HashMap();
function registerSession(sessionInfo) {
	sessionInfo.key = "session:" + uuid.v4();
	candidateSessions.set(sessionInfo.key, new Array());
	pool.open(function(redis) {
		redis.multi([
			["set", sessionInfo.key, JSON.stringify(sessionInfo)],
			["expire", sessionInfo.key, expireSec]
		]).exec(function(err, replies) {
			pool.close(redis);
			var reqs = candidateSessions.get(sessionInfo.key);
			candidateSessions.remove(sessionInfo.key);
			for (var i=0; i<reqs.length; ++i)
				reqs[i](err);
		});
	});
	
	resetNonce(this, {
		key: sessionInfo.key, 
		id: sessionInfo.id
	});
}

function unregisterSession(sessionKey) {
	pool.open(function(redis) {
		redis.del(sessionKey, function(err, replies) {
			pool.close(redis);
		});
	});
	this.clearCookie("session");
}

exports.sessionManager = function() {
	return function(req, res, next) {
		res.registerSession = registerSession;
		res.unregisterSession = unregisterSession;
		req.session = null;
		var session = null;
		if (req.cookies && req.cookies.session)
			session = JSON.parse(req.cookies.session);

		if (session == null) {
			next();
			return;
		}
		
		// 변조 여부 체크
		var invalid = false;
		if (session.key == null)
			invalid = true;
		else if (hash(session.nonce.toString()) != session.HMAC) {
			// 해커에 의해 변조된 경우 - 세션을 폐기한다.
			res.unregisterSession(session.key);
			invalid = true;
		}
		
		if (invalid) {
			console.log("invalid session");
			next(NewError(HTTPStatus.UNAUTHORIZED));
			return;
		}		
		resetNonce(res, session);
		
		// 세션키로 유저 정보 조회
		var candidate = candidateSessions.get(session.key);
		var getSessionInfo = function(err) {
			if (err) {
				console.error(err);
				next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
				return;
			}
			pool.open(function(redis) {
				var errors = [];
				var callback = function(err, reply) {
					if (err) {
						errors[0] = err;
						res.unregisterSession(session.key);
					}
					else if (reply == null) {
						res.clearCookie("session");
					}
					else {
						// success
						req.session = JSON.parse(reply);
					}
				};
				
				redis.multi([
					["get", session.key, callback],
					["expire", session.key, expireSec]
				]).exec(function(err) {
					pool.close(redis);
					errors[1] = err;
					if (errors[0] || errors[1]) {
						console.error(errors);
						next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, errors));
					}
					else {
						// success
						next();
					}
				});
			});
		}; // var getSessionInfo
		
		if (candidate)
			candidate.push(getSessionInfo);
		else
			getSessionInfo(null);
	};
}; // exports.sessionManager

//////////////////////////////////////////////////
var nn = 1;
function test() {
	pool.open(function(redis) {
		var callback = function(err, result){
			console.log(nn + "===================");
			console.log(err);
			console.log(result);
			++nn;
		};
		redis.multi([
			["get", "key1", callback],
			["expire", "key1", 10, callback]
		]).exec(function(err, replies) {
			console.log("last===================");
			console.log(err);
			console.log(replies);
			pool.closeAll();
		});
	});
	console.log("start");
}
//test();