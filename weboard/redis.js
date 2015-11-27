var Redis = require('redis');
var uuid = require('node-uuid');
var HTTPStatus = require('http-status');
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

function setSessionCookie(res, sessionInfo) {
	res.cookie("session", JSON.stringify({
		key: sessionInfo.key,
		id: sessionInfo.id,
		nonce: sessionInfo.nonce
	}));
}

function registerSession(sessionInfo, callback, retry) {
	var self = this;
	if (retry && retry >= 2) {
		// 2번이나 Unique한 UUID 발급에 실패?
		console.error("세션키 발급 실패");
		callback(null, false);
		return;
	}
	
	sessionInfo.key = "session:" + uuid.v4();
	pool.open(function(redis) {
		sessionInfo.nonce = Math.random();
		redis.setnx(sessionInfo.key, 
					JSON.stringify(sessionInfo),
					function(err, reply) {
						if (err) {
							pool.close(redis);
							callback(err, false);
						}
						else if (reply) {
							// 성공
							setSessionCookie(self, sessionInfo);
							callback(err, true);
							redis.expire(sessionInfo.key, expireSec, function() {
								pool.close(redis);
							});
						}
						else {
							// 세션키가 이미 존재하는 경우 새로운 세션키로 재시도
							pool.close(redis);
							sessionInfo.key = null;
							self.registerSession(sessionInfo,
												 callback,
												 (retry ? retry+1 : 1));
						}
					}
		);
	});
}

function resetNonce(sessionInfo, callback) {
	// 기존 세션의 nonce를 재발급 하는 경우
	var self = this;
	pool.open(function(redis) {
		// 1. watch를 걸고 키를 조회
		redis.batch([
			["watch", sessionInfo.key],
			["get", sessionInfo.key]
		]).exec(function(err, replies) {
			// 2. 유효성 검사
			var invalid = false;
			if (err)
				invalid = true;
			else if (replies && replies[1]) {
				var org = JSON.parse(replies[1]);
				if (org.nonce != sessionInfo.nonce) {
					// nonce가 불일치한 경우 해킹시도로 판단하고 세션 폐기
					self.unregisterSession(sessionInfo.key);
					invalid = true;
				}
			}
			else {
				// 다른 요청에 의해 세션이 폐기된 경우 (해킹시도로 인한 nonce 불일치 등)
				// 이 갱신 요청도 취소하고 쿠키 폐기
				self.unregisterSession(sessionInfo.key);
				invalid = true;
			}
			
			if (invalid) {
				redis.unwatch(function() {
					pool.close(redis);
				});
				callback(err, false);
				return;
			}
			
			// 3. 유효한 세션인 경우 nonce를 갱신
			sessionInfo.nonce = Math.random();
			redis.multi()
				.setex(sessionInfo.key, expireSec, JSON.stringify(sessionInfo))
				.exec(function(err, replies) {
					pool.close(redis);
					if (err)
						callback(err, false);
					else if (replies) {
						// 성공
						setSessionCookie(self, sessionInfo);
						callback(err, true);
					}
					else {
						// 트랜잭션 중 다른 요청에서 세션을 수정 또는 폐기한 경우
						// 해킹시도로 판단하고 세션 폐기
						self.unregisterSession(sessionInfo.key);
						callback(err, false);
					}
				});
		});
	});
} // function registerSession

function unregisterSession(sessionKey) {
	if (sessionKey) {
		pool.open(function(redis) {
			redis.del(sessionKey, function(err, replies) {
				pool.close(redis);
			});
		});
	}
	this.clearCookie("session");
}

exports.sessionManager = function() {
	return function(req, res, next) {
		res.registerSession = registerSession;
		res.resetNonce = resetNonce;
		res.unregisterSession = unregisterSession;
		req.session = null;

		var sessionCookie = null;
		if (req.cookies && req.cookies.session)
			sessionCookie = JSON.parse(req.cookies.session);

		if (sessionCookie == null) {
			next();
			return;
		}

		if (sessionCookie.key == null) {
			console.log("invalid session");
			next(NewError(HTTPStatus.UNAUTHORIZED));
			return;
		}
		
		// 세션 정보 조회
		pool.open(function(redis) {
			redis.batch([
				["get", sessionCookie.key],
				["expire", sessionCookie.key, expireSec]
			]).exec(function(err, replies) {
				pool.close(redis);
				if (err) {
					console.error(errors);
					next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, errors));
				}
				else {
					if (replies==null || replies[0]==null) {
						// 세션이 존재하지 않는 경우 (만료)
						res.unregisterSession();
						next();
					}
					else {
						// nonce 검사
						var session = JSON.parse(replies[0]);
						if (session.nonce == sessionCookie.nonce) {
							// 세션 인증 성공
							req.session = session;
							next();
						}
						else {
							// 불일치 시 세션 폐기
							res.unregisterSession(session.key);
							next(NewError(HTTPStatus.UNAUTHORIZED));
						}	
					}
				}
			});
		});
	};
}; // exports.sessionManager
