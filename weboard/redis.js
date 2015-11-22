var Redis = require('redis');
var uuid = require('node-uuid');
var HashMap = require('hashmap');
var NewError = require("./error");

var expireSec = 60;

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

	var redis;
	if (this.pool.length > 0)
		redis = this.pool.shift();
	else {
		if (this.stage.length >= this.options.max)
			return;
		redis = NewClient(this);
	}
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
	if (i == -1)
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

exports.createPool = function(options) {
	return new Pool(options);
};

var pool = new Pool({
	host: "192.168.88.130",
	port: 6379
});

var candidateSessions = new HashMap();
function registerSession(sessionInfo) {
	var expire = new Date();
	expire.setSeconds(expire.getSeconds() + expireSec);
	sessionInfo.expire = JSON.stringify(expire);
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
	this.cookie("sessionKey", sessionInfo.key, {maxAge: (expireSec*1000)});
}

exports.sessionManager = function(req, res, next) {
	res.registerSession = registerSession;
	var sessionKey = req.cookies.sessionKey;
	if (sessionKey) {
		var candidate = candidateSessions.get(sessionKey);
		var getSessionInfo = function(err) {
			if (err) {
				next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, err));
				return;
			}
			pool.open(function(redis) {
				var error = null;
				var callback = function(err, reply) {
					if (err) {
						error = err;
						return;
					}
					try {
						req.session = JSON.parse(reply);
					}
					catch(e) {
						res.clearCookie("sessionKey");
						req.session = null;
					}
				};
				
				redis.multi([
					["get", sessionKey, callback],
					["expire", sessionKey, expireSec]
				]).exec(function() {
					pool.close(redis);
					if (error)
						next(NewError(HTTPStatus.INTERNAL_SERVER_ERROR, error));
					else
						next();
				});
			});
		};
		if (candidate)
			candidate.push(getSessionInfo);
		else
			getSessionInfo(null);
		return;
	}
	req.session = null;
	next();
};

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