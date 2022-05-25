#pragma once

#ifdef _DEBUG 
#define DEBUG_NEW new(__FILE__, __LINE__) 
#else 
#define DEBUG_NEW new 
#endif 
#define new DEBUG_NEW 

#include <string>  
#include <vector>  
#include <list>  
  
#include "../sqlite/sqlite3.h"  
  
class SQLiteStatement {  
  private:  
    // SQLiteStatement's ctor only to be called by SQLiteWrapper  
    friend class SQLiteWrapper;  
    SQLiteStatement(std::string const& statement, sqlite3* db);  
  
  public:  
    SQLiteStatement();

    void operator delete(void *p) {
    }
    void operator delete[](void *p) {
    }

    void clear() {
      return ::operator delete(this);
    }

    enum dataType {  
      INT = SQLITE_INTEGER,  
      FLT = SQLITE_FLOAT  ,  
      TEXT = SQLITE_TEXT   ,  
      BLB = SQLITE_BLOB   ,  
      NUL = SQLITE_NULL   ,  
    };  
  
    dataType DataType(int pos_zero_indexed);  
  
    int         ValueInt   (int pos_zero_indexed);  
    CString ValueString(int pos_zero_indexed);  
  
//    SQLiteStatement(const SQLiteStatement&);  
   ~SQLiteStatement();  
  
    //SQLiteStatement& operator=(SQLiteStatement const&);  
  
    bool Bind    (int pos_zero_indexed, std::string const& value);  
    bool Bind    (int pos_zero_indexed, double             value);  
    bool Bind    (int pos_zero_indexed, int                value);  
    bool BindNull(int pos_zero_indexed);  
  
    bool Execute();  
  
    bool NextRow();  
  
    /*   Call Reset if not depending on the NextRow cleaning up. 
         For example for select count(*) statements*/  
    bool Reset();  
  
    bool RestartSelect();  
	int GetColCount();
	CString GetColName(int col);
  private:  
    //void DecreaseRefCounter();  
  
    //int* ref_counter_; // not yet used...  
    sqlite3_stmt* stmt_;  
};  
  

typedef std::list< SQLiteStatement* > list_t;

// 为了便于管理禁止使用该类的全局变量
class SQLiteWrapper {  
  public:  
    SQLiteWrapper();  
	~SQLiteWrapper() ;
    bool Open(const char* db_file);
    class ResultRecord {  
      public:  
        std::vector<std::string> fields_;  
    };  
  
    class ResultTable {  
      friend class SQLiteWrapper;  
      public:  
        ResultTable() : ptr_cur_record_(0) {}  
      
        std::vector<ResultRecord> records_;  
      
         ResultRecord* next() {  
          if (ptr_cur_record_ < records_.size()) {  
            return &(records_[ptr_cur_record_++]);  
          }  
          return 0;  
        }  
      
      private:  
         void reset() {  
           records_.clear();  
           ptr_cur_record_=0;  
         }  
      
      private:  
        unsigned int ptr_cur_record_;  
    };  
  
   
    bool Writedb(std::string const& stmt);  
    SQLiteStatement* Readdb(std::string const& stmt);
	 SQLiteStatement* Readdb(CString path, std::string const& stmt); 

   SQLiteStatement* stmt_;
   list_t list_;

	int GetColCount(char* sql);
	int GetRowCount(char* sql);
	void Close();
    std::string LastError();  
	bool Begin   ();  
	bool Commit  ();  
	bool Rollback(); 
  
  //static bool CloseGlobalDB(void);
    // Transaction related  

  
  private:  
	  bool SelectStmt           (std::string const& stmt, ResultTable& res);  
 
    static int SelectCallback(void *p_data, int num_fields, char **p_fields, char **p_col_names);  
  
    sqlite3* db_;  
}; 