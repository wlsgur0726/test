var __this = require("./srcinfo.js");
var odbc = require("odbc").Database;
var constr = "Driver=MySQL ODBC 5.3 Unicode Driver;"
		   + "Database=weboard;"
		   + "Server=192.168.88.130;"
		   + "User=root; Password=password;"
		   + "charset=utf8;";

function checkError(err, db, lastCallback) {
	if (err) {
		if (lastCallback)
			lastCallback(err, 0, 0);
		else
			console.log("[" + __this.caller.line() + "] DBError : "  + err);
		db.close(null);
		return true;
	}
	return false;
};

function prepareAndExecute(query, params, fetchCallback, lastCallback) {
	var db = new odbc();
	
	db.open(constr, function(err) {
		if (checkError(err, db, lastCallback))
			return;
		
		db.prepare(query, function(err, stmt) {
			if (checkError(err, db, lastCallback))
				return;
			
			stmt.execute(params, function(err, result) {
				if (checkError(err, db, lastCallback))
					return;
				
				var resNum = 0;
				var recNum = 0;
				var fetchAfter = function (err, data) {
					if (checkError(err, db, lastCallback))
						return;
					if (data == null) {
						if (result.moreResultsSync() == false) {
							db.close(null);
							if (lastCallback)
								lastCallback(null, resNum, recNum);
							return;
						}
						++resNum;
					}
					else {
						if (fetchCallback)
							fetchCallback(resNum, recNum, data);
						++recNum;
					}
					result.fetch(fetchAfter);
				};
				result.fetch(fetchAfter);
			});
		});
	});
};

exports.getContentList = function(start, count, fetchCallback, lastCallback) {
	prepareAndExecute("call GetContentList(?,?)",
	                  [start, count],
					  fetchCallback,
					  lastCallback);
};

exports.getContent = function(number, callback) {
	var content = null;
	var comments = new Array();
	prepareAndExecute("select content from board where number=?",
					  [number],
					  function(resNum, recNum, data) {
						  if (resNum == 0)
							  content = data;
						  else
							  comments.push(data);
					  },
					  function(err, resCnt, recCnt) {
						  callback(content, comments);
					  });
};

exports.addContent = function(title, writer, content, callback) {
	var number = null;
	prepareAndExecute("call AddContent(?,?,?)",
					  [title, writer, content],
					  function(resNum, recNum, data) {
						  number = data;
					  },
					  function(err, resCnt, recCnt) {
						  if (err || number==null) {
							  callback(false, -1);
						  }
						  else 
							  callback(true, number.number);
					  });
};

exports.addComment = function(owner, writer, comment, callback) {
	var number = null;
	prepareAndExecute("call AddComment(?,?,?)",
					  [owner, writer, comment],
					  function(resNum, recNum, data) {
						  number = data;
					  },
					  function(err, resCnt, recCnt) {
						  if (err || number==null)
							  callback(false, -1);
						  else 
							  callback(true, number.number);
					  });
}

exports.signUp = function(id, pw, callback) {
	prepareAndExecute("insert into user(id, password) values(?, ?)",
	                  [id, pw],
					  null,
					  function(err, resCnt, recCnt) {
						  var success = true;
						  if (err)
							  success = false;
						  callback(id, success);
					  });
};

exports.signIn = function(id, pw, callback) {
	var userinfo = null;
	prepareAndExecute("select * from user where id=? and password=?",
	                  [id, pw],
					  function(resNum, recNum, data) {
						  userinfo = data;
					  },
					  function(err, resCnt, recCnt) {
						  callback(id, userinfo);
					  });
};