#include "StdAfx.h"
//#include "Log.h"


//sqlite3* g_db_path = NULL;  //CString datapath;
//sqlite3* g_db_app_path = NULL;  //CString ProductAppPath;

int dbopen = 0;
int dbclose = 0;


CCriticalSection ccDatabase;

#include "database.h"  
// TODO: raus  
#include <windows.h>  

SQLiteWrapper::SQLiteWrapper() : db_(0) {  
  ccDatabase.Lock();
  stmt_ = NULL;
}  
SQLiteWrapper::~SQLiteWrapper()  {

   //if (stmt_) {
   //  stmt_->clear();
   //  stmt_ = NULL;
   //}

   while (!list_.empty()) {
     stmt_ = list_.front();
     if (stmt_) {
       stmt_->clear();
       stmt_ = NULL;
     }
     list_.pop_front();
   }

   if (db_)
   {
      //sqlite3_close(db_);
     if (SQLITE_BUSY == sqlite3_close(db_))
     {
       AfxMessageBox(_T("数据库释放不了"));
     }
     db_ = NULL;
   }


   ccDatabase.Unlock();
}
bool SQLiteWrapper::Open( const char* db_file) {
	CStringW sql2 = CA2W(db_file); 

  if (db_ == NULL && db_file !="")
  {
    CStringW tmp = CA2W(db_file);
    if (sqlite3_open(CW2A(tmp, CP_UTF8)/*datapath.GetBuffer()*/, &db_) != SQLITE_OK) {
      AfxMessageBox(_T("数据库打开失败"));
      exit(0); return false;
    }
  }

  //db_ = g_db_path;
 
	return true;  
}   

bool SQLiteWrapper::SelectStmt(std::string const& stmt, ResultTable& res) {  
	char *errmsg;  
	int   ret;  

	res.reset();  

	ret = sqlite3_exec(db_, stmt.c_str(), SelectCallback, static_cast<void*> (&res), &errmsg);  

	if (ret != SQLITE_OK) {  
		return false;  
	}  
	return true;  
	//  if (ret != SQLITE_OK) {  
	//    std::cout << stmt << " [" << errmsg << "]" << std::endl;  
	//  }  
}  

// TODO parameter p_col_names  
int SQLiteWrapper::SelectCallback(void *p_data, int num_fields, char **p_fields, char** p_col_names) {  
	ResultTable* res = reinterpret_cast<ResultTable*>(p_data);  

	ResultRecord record;  

#ifdef SQLITE_WRAPPER_REPORT_COLUMN_NAMES  
	// Hubert Castelain: column names in the first row of res if res is empty  

	if(res->records_.size()==0) {  
		ResultRecord col_names;  

		for(int i=0; i < num_fields; i++) {  
			if(p_fields[i]) col_names.fields_.push_back (p_col_names[i]);  
			else  
				col_names.fields_.push_back("(null)"); // or what else ?  
		}  
		res->records_.push_back(col_names);  
	}  
#endif  

	for(int i=0; i < num_fields; i++) {  
		// Hubert Castelain: special treatment if null  
		if (p_fields[i]) record.fields_.push_back(p_fields[i]);  
		else             record.fields_.push_back("<null>");  
	}  

	res->records_.push_back(record);  

	return 0;  
}  

SQLiteStatement* SQLiteWrapper::Readdb(std::string const& statement) { 
	//SQLiteStatement* stmt;  
	CStringW sql2 = CA2W(statement.c_str());

	//ret = sqlite3_exec(db_, CW2A(sql2, CP_UTF8), 0, 0, &errmsg);
	try {  
 /*   if (stmt_) {
      stmt_->clear();
      stmt_ = NULL;
    }*/
		stmt_ = new SQLiteStatement(std::string(CW2A(sql2, CP_UTF8)) , db_);  
    list_.push_back(stmt_);
		return stmt_;  
	}  
	catch (const char* e) {  
		return 0;  
	}  
}  

SQLiteStatement* SQLiteWrapper::Readdb( CString path, std::string const& statement )
{
#ifdef UNICODE 
  //unicode
	if (!Open(CW2A(path, CP_UTF8)))
  {  
		AfxMessageBox(_T("数据库打开失败")); 
	}
#else 
  //多字节
  if (!Open(path.GetBuffer()))
  {
    AfxMessageBox(_T("数据库打开失败"));
  }
  path.ReleaseBuffer();
#endif


	//SQLiteStatement* stmt;  
	CStringW sql2 = CA2W(statement.c_str());
	//ret = sqlite3_exec(db_, CW2A(sql2, CP_UTF8), 0, 0, &errmsg);
	try {  
    //if (stmt_) {
    //  stmt_->clear();
    //  stmt_ = NULL;
    //}
		stmt_ = new SQLiteStatement(std::string(CW2A(sql2, CP_UTF8)) , db_);  
    list_.push_back(stmt_);
		//Close();
		return stmt_;  
	}  
	catch (const char* e) {  
    //AfxMessageBox(_T("数据库catch异常"));
		return 0;  
	} 


}

int SQLiteWrapper::GetColCount(char* sql)
{
	char *zErrMsg = 0;
	int rc;
	char **dbResult;
	int nRow, nColumn;
	CStringW sql2 = CA2W(sql);
	  sqlite3_get_table( db_, CW2A(sql2, CP_UTF8), &dbResult, &nRow, &nColumn, &zErrMsg );
	return nColumn;
}

CString SQLiteStatement::GetColName(int col)
{
	const char* str = sqlite3_column_name(stmt_, col);
	std::wstring name2 =CA2W( str, CP_UTF8);
	CString cstr(name2.c_str());
	return cstr;
}
SQLiteStatement::SQLiteStatement(std::string const& statement, sqlite3* db) {  
	if ( sqlite3_prepare(  
		db,   
		statement.c_str(),  // stmt  
		-1,                  // If than zero, then stmt is read up to the first nul terminator  
		&stmt_,  
		0                   // Pointer to unused portion of stmt  
		)  
		!= SQLITE_OK) {  
			throw sqlite3_errmsg(db);  
	}  

	if (!stmt_) {  
		throw "stmt_ is 0";  
	}  
}  

SQLiteStatement::~SQLiteStatement() {  
	// Hubert Castelain 28/8/2005  
	// Prevent the database remaining locked after some statement.  
	// syntax: int sqlite3_finalize(sqlite3_stmt *pStmt);  
	if(stmt_) sqlite3_finalize(stmt_);  
}  

SQLiteStatement::SQLiteStatement() :  
stmt_       (0)  
{  
}  

bool SQLiteStatement::Bind(int pos_zero_indexed, std::string const& value) {  
	if (sqlite3_bind_text (  
		stmt_,  
		pos_zero_indexed+1,  // Index of wildcard  
		value.c_str(),  
		value.length(),      // length of text  
		SQLITE_TRANSIENT     // SQLITE_TRANSIENT: SQLite makes its own copy  
		)  
		!= SQLITE_OK) {  
			return false;  
	}  
	return true;  
}  

bool SQLiteStatement::Bind(int pos_zero_indexed, double value) {  
	if (sqlite3_bind_double(  
		stmt_,  
		pos_zero_indexed+1,  // Index of wildcard  
		value  
		)  
		!= SQLITE_OK) {  
			return false;  
	}  
	return true;  
}  

bool SQLiteStatement::Bind(int pos_zero_indexed, int value) {  
	if (sqlite3_bind_int(  
		stmt_,  
		pos_zero_indexed+1,  // Index of wildcard  
		value   
		)  
		!= SQLITE_OK) {  
			return false;  
	}  
	return true;  
}  

bool SQLiteStatement::BindNull(int pos_zero_indexed) {  
	if (sqlite3_bind_null(  
		stmt_,  
		pos_zero_indexed+1  // Index of wildcard  
		)  
		!= SQLITE_OK) {  
			return false;  
	}  
	return true;  
}  

bool SQLiteStatement::Execute() {  
	int rc = sqlite3_step(stmt_);  
	if (rc == SQLITE_BUSY) {  
		::MessageBox(0, _T("SQLITE_BUSY"), 0, 0);   
		return false;  
	}  
	if (rc == SQLITE_ERROR) {  
		::MessageBox(0, _T("SQLITE_ERROR"), 0, 0);   
		return false;  
	}  
	if (rc == SQLITE_MISUSE) {  
		::MessageBox(0, _T("SQLITE_ERROR"), 0, 0);   
		return false;  
	}  
	if (rc != SQLITE_DONE) {  
		//sqlite3_reset(stmt_);  
		return false;  
	}  
	sqlite3_reset(stmt_);  
	return true;  
}  

SQLiteStatement::dataType SQLiteStatement::DataType(int pos_zero_indexed) {  
	return dataType(sqlite3_column_type(stmt_, pos_zero_indexed));  
}  

int SQLiteStatement::ValueInt(int pos_zero_indexed) {  
	return sqlite3_column_int(stmt_, pos_zero_indexed);  
}  

CString SQLiteStatement::ValueString(int pos_zero_indexed) {  
	//std::string str = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt_, pos_zero_indexed)));
	const char *str =reinterpret_cast<const char*>(sqlite3_column_text(stmt_, pos_zero_indexed)); 
	if (str==NULL)
	{
		return _T("");
	}
	std::wstring name2 =CA2W( str, CP_UTF8);
	CString cstr(name2.c_str());
	return cstr;
}  

bool SQLiteStatement::RestartSelect() {  
	sqlite3_reset(stmt_);  
	return true;  
}  

bool SQLiteStatement::Reset() {  
	int rc = sqlite3_step(stmt_);  

	sqlite3_reset(stmt_);  

	if (rc == SQLITE_ROW) return true;  
	return false;  
}  
int SQLiteStatement::GetColCount()
{
	return sqlite3_column_count(stmt_);
	
}
bool SQLiteStatement::NextRow() {  
	int rc = sqlite3_step(stmt_);  
	if (rc == SQLITE_ROW   ) {  
		return true;  
	}  
	if (rc == SQLITE_DONE  ) {  
		sqlite3_reset(stmt_);  
		return false;  
	}   
	else if (rc == SQLITE_MISUSE) {  
		::MessageBox(0,  _T("SQLiteStatement::NextRow SQLITE_MISUSE"), 0, 0);  
	}   
	else if (rc == SQLITE_BUSY  ) {  
		::MessageBox(0, _T("SQLiteStatement::NextRow SQLITE_BUSY"), 0, 0);  
	}   
	else if (rc == SQLITE_ERROR ) {  
		::MessageBox(0, _T("SQLiteStatement::NextRow SQLITE_ERROR"), 0, 0);  
	}  
	return false;  
}  

bool SQLiteWrapper::Writedb(std::string const& stmt) {  
	char *errmsg;  
	int   ret;  
	CStringW sql2 = CA2W(stmt.c_str());
#if 1
	ret = sqlite3_exec(db_, CW2A(sql2, CP_UTF8), 0, 0, &errmsg);  
#else
	int nCount=0;
	do 
	{
		ret = sqlite3_exec(db_, CW2A(sql2, CP_UTF8), 0, 0, &errmsg);  

		if (ret == SQLITE_BUSY)
		{
			Sleep(1);
			continue;
		}
		break;
	} while (1);

#endif
	if(ret != SQLITE_OK) {  
		//LOG(errmsg);
		return false;  
	}  
	return true;  

	//if(ret != SQLITE_OK) {  
	//  std::cout << stmt << " [" << errmsg << "]" << std::endl;  
	//}  
}  
void SQLiteWrapper::Close()
{
  //if(db_)
	  //sqlite3_close(db_);
	db_ = 0;
	//ccDatabase.Unlock();
  //OPEN_STACK_TRACK1
}
std::string SQLiteWrapper::LastError() {  
	return sqlite3_errmsg(db_);  
}  

bool SQLiteWrapper::Begin() {  
	return Writedb("begin");  
}  

bool SQLiteWrapper::Commit() {  
	return Writedb("commit");  
}  

bool SQLiteWrapper::Rollback() {  
	return Writedb("rollback");  
}

//bool SQLiteWrapper::CloseGlobalDB(void)
//{
//  if (SQLITE_BUSY == sqlite3_close(g_db_path))
//  {
//    AfxMessageBox(_T("数据库释放不了"));
//    return false;
//  }
//  if (SQLITE_BUSY == sqlite3_close(g_db_app_path))
//  {
//    AfxMessageBox(_T("APP数据库释放不了"));
//    return false;
//  }
//  g_db_path = NULL;
//  g_db_app_path = NULL;
//  return true;
//}

int SQLiteWrapper::GetRowCount( char* sql )
{
	char *zErrMsg = 0;
	int rc;
	char **dbResult;
	int nRow, nColumn;
	CStringW sql2 = CA2W(sql);
	sqlite3_get_table( db_, CW2A(sql2, CP_UTF8), &dbResult, &nRow, &nColumn, &zErrMsg );
	return nRow;
}
