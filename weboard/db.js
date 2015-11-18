var __this = require("./srcinfo.js");
var Pool = require("odbc").Pool;
var pool = new Pool();
/*
var constr = "Driver=MySQL ODBC 5.3 Unicode Driver;"
		   + "Database=weboard;"
		   + "Server=192.168.88.130;"
		   + "User=root; Password=password;"
		   + "charset=utf8;";
*/
var constr = "Driver=SQL Server Native Client 11.0;"
		   + "Server=.;"
		   + "Database=weboard;"
		   + "Trusted_Connection=Yes;";

		   
// 너무 많은 커넥션이 생기지 않도록 통제
const ConnectionLimit = 100;
var connectionController = {
	queue: new Array(),
	counter: 0,
	request: function(work) {
		if (this.queue.length == 0 && this.counter < ConnectionLimit) {
			++this.counter;
			return work();
		}
		this.queue.push(work);
	},
	complete: function() {
		--this.counter;
		if (this.queue.length == 0 || this.counter >= ConnectionLimit)
			return;
		
		++this.counter;
		var work = this.queue.shift();
		work();
	}
};
		   
function checkError(err, db, lastCallback) {
	if (err) {
		if (lastCallback)
			lastCallback(err, new Array());
		else
			console.log("[" + __this.caller.line() + "] DBError : "  + err);
		db.complete();
		return true;
	}
	return false;
};

function prepareAndExecute(query, params, fetchCallback, lastCallback) {
	connectionController.request(function() {
		pool.open(constr, function(err, db) {
			db.stmt = null;
			db.result = null;
			db.complete = function() {
				if (db.result != null) {
					db.result.closeSync();
					db.result = null;
				}
				if (db.stmt != null) {
					db.stmt.closeSync();
					db.stmt = null;
				}
				db.close(function(){});
				connectionController.complete();
			};
			if (checkError(err, db, lastCallback))
				return;
			
			db.prepare(query, function(err, stmt) {
				db.stmt = stmt;
				if (checkError(err, db, lastCallback))
					return;
				
				stmt.execute(params, function(err, result) {
					db.result = result;
					if (checkError(err, db, lastCallback))
						return;
					
					var resNum = 0;
					var afterFetch = function (err, list) {
						if (checkError(err, db, lastCallback))
							return;

						if (fetchCallback)
							fetchCallback(resNum, list);

						if (result.moreResultsSync()) {
							++resNum;
							result.fetchAll(afterFetch);
						}
						else {
							db.complete();
							if (lastCallback)
								lastCallback(null);
						}
					};
					result.fetchAll(afterFetch);
				});
			});
		});
	});
};

exports.getContentList = function(start, count, callback) {
	var contents = null;
	prepareAndExecute("call sp_getContentList(?,?)",
	                  [start, count],
					  function(resNum, list) {
						  contents = list;
					  },
					  callback ? function(err){
						  callback(err, contents);
					  } : null);
};

exports.getContent = function(number, callback) {
	var content = null;
	var comments = null;
	prepareAndExecute("{call sp_getContent(?)}",
					  [number],
					  function(resNum, list) {
						  if (resNum == 0)
							  content = list[0];
						  else
							  comments = list;
					  },
					  callback ? function(err) {
						  if (err) console.log(err);
						  callback(content.content, comments);
					  } : null);
};

exports.addContent = function(title, writer, content, callback) {
	var number = null;
	prepareAndExecute("{call sp_addContent(?,?,?)}",
					  [title, writer, content],
					  function(resNum, list) {
						  if (list == null)
							  return;
						  for (var i=0; i<list.length; ++i) {
							  if (list[i].number != null) {
								  number = list[i].number;
								  break;
							  }
						  }
					  },
					  callback ? function(err) {
						  if (err || number==null)
							  callback(err, -1);
						  else 
							  callback(err, number.number);
					  } : null);
};

exports.addComment = function(owner, writer, comment, callback) {
	var number = null;
	prepareAndExecute("{call sp_addComment(?,?,?)}",
					  [owner, writer, comment],
					  function(resNum, list) {
						  if (list == null)
							  return;
						  for (var i=0; i<list.length; ++i) {
							  if (list[i].number != null) {
								  number = list[i].number;
								  break;
							  }
						  }
					  },
					  callback ? function(err) {
						  if (err || number==null)
							  callback(err, -1);
						  else 
							  callback(err, number);
					  } : null);
}

exports.signUp = function(id, pw, callback) {
	prepareAndExecute("insert into tb_user(id, password) values(?, ?)",
	                  [id, pw],
					  null,
					  callback ? function(err) {
						  if (err) console.log(err);
						  var success = (err == null);
						  callback(id, success);
					  } : null);
};

exports.signIn = function(id, pw, callback) {
	var userinfo = null;
	prepareAndExecute("select * from tb_user where id=? and password=?",
	                  [id, pw],
					  function(resNum, list) {
						  userinfo = list[0];
					  },
					  callback ? function(err) {
						  callback(id, userinfo);
					  } : null);
};