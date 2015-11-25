var __this = require("./srcinfo.js");
var Pool = require("odbc").Pool;
var pool = new Pool();

if (0) {
var constr = "Driver=MySQL ODBC 5.3 Unicode Driver;"
		   + "Database=weboard;"
		   + "Server=192.168.88.130;"
		   + "User=root; Password=password;"
		   + "charset=utf8;";
}
else {
var constr = "Driver=SQL Server Native Client 11.0;"
		   + "Server=.;"
		   + "Database=weboard;"
		   + "Trusted_Connection=Yes;";
}
		   
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
			lastCallback(err);
		else
			console.log("[" + __this.caller().line() + "] DBError : "  + err);
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

exports.prepareAndExecute = prepareAndExecute;

exports.getContentList = function(start, count, callback) {
	var contents = null;
	prepareAndExecute("{call sp_getContentList(?,?)}",
	                  [start, count],
					  function(resNum, list) {
						  if (resNum == 0)
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
						  if (resNum == 0) {
							  if (list.length > 0 && typeof list[0].content != undefined)
								  content = list[0];
						  }
						  else {
							  if (content != null)
								  comments = list;
						  }
					  },
					  callback ? function(err) {
						  callback(err, content, comments);
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
						  callback(err, number);
					  } : null);
};

exports.editContent = function(number, title, writer, content, callback) {
	if (title==null && content==null) {
		callback(null, number);
		return;
	}
	
	var query;
	var params;
	if (title!=null && content!=null) {
		query = "update tb_board set title=?, content=? where number=? and writer=?";
		params = [title, content, number, writer];
	}
	else if (title!=null) {
		query = "update tb_board set title=? where number=? and writer=?";
		params = [title, number, writer];
	}
	else {
		query = "update tb_board set content=? where number=? and writer=?";
		params = [content, number, writer];
	}
	
	prepareAndExecute(query, params, null,
					  callback ? function(err) {
						  callback(err, number);
					  } : null);
};

exports.deleteContent = function(number, writer, callback) {
	prepareAndExecute("delete from tb_board where number=? and writer=?", 
					  [number, writer],
					  null,
					  callback ? function(err) {
						  callback(err, number);
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
						  callback(err, number);
					  } : null);
}

exports.deleteComment = function(number, writer, callback) {
	prepareAndExecute("delete from tb_comment where number=? and writer=?", 
					  [number, writer],
					  null,
					  callback ? function(err) {
						  callback(err, number);
					  } : null);
};

exports.signUp = function(id, pw, callback) {
	prepareAndExecute("insert into tb_user(id, password) values(?, ?)",
	                  [id, pw],
					  null,
					  callback ? function(err) {
						  var success = (err == null);
						  callback(err, id, success);
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
						  callback(err, id, userinfo);
					  } : null);
};