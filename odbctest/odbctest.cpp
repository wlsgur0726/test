#include <sql.h>
#include <sqlext.h>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include "../../asd/asd/asd_core/include/asd/exception.h"
#include "../../asd/asd/asd_core/include/asd/string.h"

struct OdbcHandle
{
	virtual ~OdbcHandle() = 0;
	virtual SQLHANDLE GetHandle() const = 0;
	virtual SQLSMALLINT GetHandleType() const = 0;
	SQLHENV m_handle = SQL_NULL_HENV;

public:
	OdbcHandle()
	{
		SQLRETURN r = SQLAllocHandle(SQL_HANDLE_ENV,
									 SQL_NULL_HANDLE,
									 &m_handle);
		if (r != SQL_SUCCESS) {
			if (r != SQL_SUCCESS_WITH_INFO) {

			}
		}
		assert(m_handle != SQL_NULL_HENV);
	}

	virtual SQLHANDLE GetHandle() const
	{
		return m_handle;
	}
};

struct OdbcError
{
	SQLCHAR stateCode[6] = {0,};
	SQLINTEGER errNo;
	SQLSMALLINT errMsgLen;
	SQLCHAR errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
	SQLRETURN result;

};

void GetErrorList(OdbcHandle* handle)
{
	SQLCHAR stateCode[6] = {0,};
	SQLINTEGER errNo;
	SQLSMALLINT errMsgLen;
	SQLCHAR errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
	SQLRETURN result;
}


int main()
{
	return 0;
}
