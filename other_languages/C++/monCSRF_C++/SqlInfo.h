/*********************************************************
SqlInfo is a class that:
1) Initializes with a query.
     ex: SqlInfo *query_info = new SqlInfo("UPDATE blalblablalbalbal .... ");
2) Parses this query and save into a variable (query_info).
3) Has a method to compare this object:
     query_info.compare(another_query_info)
4) If they are the same (without the input values), true is returned.
5) All the GET queries with UPDATE, INSERT INTO and DELETE FROM in the log file can be compared to those that are in the whitelist.
*********************************************************/

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <pcrecpp.h>
#include "ParserUtil.h"

using namespace std;
class SqlInfo;

class SqlInfo {

  public:

    SqlInfo(string sql);
    void parse();

    bool compare(SqlInfo* sql_info_to_compare);

    string getSqlCommand();
    string getSqlCommandType();
    string getSqlTableName();
    string getSqlColumns();
    string getSqlWhereExp();
    string getSqlValueExp();

  private:

    bool parsed;
    string sql_command;
    string sql_command_type;
    string sql_table_name;
    string sql_columns;
    string sql_where_exp;
    string sql_value_exp;

    void print_debug_info();

    string normalize_sql(string sql);

    void parse_update_command(string sql);
    void parse_delete_command(string sql);
    void parse_create_command(string sql);

    string remove_values_from_update_fields_list(string fields_list);
    string remove_values_from_where(string where_field);
};
