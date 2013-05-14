#include "SqlInfo.h"
#include <string>


// ---------------------------------------
SqlInfo::SqlInfo(string sql) {
  this->sql_command = this->normalize_sql(sql);
  this->parsed = false;
  this->parse();
}


// ----  Returns true if the sql_info_to_compare has the same query syntax of this object's query
bool SqlInfo::compare(SqlInfo* sql_info_to_compare) {
  if (this->sql_command_type.compare(sql_info_to_compare->getSqlCommandType()) == 0){  // Compares if it's the same command type
    if (this->sql_table_name.compare(sql_info_to_compare->getSqlTableName()) == 0){    // Compares if it's on the same table
      if (this->sql_columns.compare(sql_info_to_compare->getSqlColumns()) == 0) {      // Compares if it's the same updated columns
        if (this->sql_where_exp.compare(sql_info_to_compare->getSqlWhereExp()) == 0){  // Compares if it's the same where conditions
          return true;
        }
        else if (this->sql_value_exp.compare(sql_info_to_compare->getSqlValueExp()) == 0){ // compare if it's the same value for insert into
          return true;
        }
      }
    }
  }
  return false;
}



//---------------------------------
string SqlInfo::getSqlCommand() {
  return this->sql_command;
}


// ---------------------------------
string SqlInfo::getSqlCommandType(){
  return this->sql_command_type;
}


// ---------------------------------
string SqlInfo::getSqlTableName(){
  return this->sql_table_name;
}


// --------------------------------
string SqlInfo::getSqlColumns(){
  return this->sql_columns;
}


// --------------------------------
string SqlInfo::getSqlWhereExp(){
  return this->sql_where_exp;
}


// ------------------------------
string SqlInfo::getSqlValueExp(){
  return this->sql_value_exp;
}

// -------- Parses the SQL into the SqlInfo object
void SqlInfo::parse() {
  if (this->parsed)
    return;

  //------ Gets the query type (update, delete, ...)
  pcrecpp::RE *command_type_regexp = new pcrecpp::RE("^([A-Za-z]+) ");
  command_type_regexp->PartialMatch(this->sql_command, &this->sql_command_type);
  string temp_sql = this->sql_command;
  command_type_regexp->Replace("", &temp_sql);

  //--------  Parse the query with the correct parser
  if (this->sql_command_type.compare("UPDATE") == 0) {
    this->parse_update_command(temp_sql);
  }
  else if (this->sql_command_type.compare("INSERT") == 0) {
    this->parse_create_command(temp_sql.c_str());
  }
  else if (this->sql_command_type.compare("DELETE") == 0) {
    this->parse_delete_command(temp_sql);
  }
  this->parsed = true;
  return;
}

//------- Removes extra whitespaces and newlines from the query
string SqlInfo::normalize_sql(string sql) {
  pcrecpp::RE("\n").GlobalReplace(" ", &sql);
  pcrecpp::RE("\r").GlobalReplace(" ", &sql);
  pcrecpp::RE("\t").GlobalReplace(" ", &sql);
  pcrecpp::RE("( )+").GlobalReplace(" ", &sql);
  return sql;
}


// -----------------------------------------
// --------- PARSE UPDATE COMMAND ----------
// -----------------------------------------
void SqlInfo::parse_update_command(string sql) {
  pcrecpp::RE *parser;

  // ------- Getting the table name
  parser = new pcrecpp::RE("^([_A-Za-z]+) SET ");
  parser->PartialMatch(sql, &this->sql_table_name);
  parser->Replace("", &sql);

  // ------ Getting column list
  string fields_list;
  parser = new pcrecpp::RE("^(.*) WHERE ");
  parser->PartialMatch(sql, &fields_list);
  this->sql_columns = this->remove_values_from_update_fields_list(fields_list);
  parser->Replace("", &sql);

  //-------- Getting WHERE arguments
  this->sql_where_exp = remove_values_from_where(sql);

  // ------- Debug info
  this->print_debug_info();

 }

// ------------------------------------------------
//-------- PARSE INSERT INTO COMMAND --------------
//-------------------------------------------------
void SqlInfo::parse_create_command(string sql) {
  pcrecpp::RE *parser;

  parser = new pcrecpp::RE("(INTO )?");
  parser->Replace("", &sql);


  // --------- Getting the table name
  parser = new pcrecpp::RE("^([_A-Za-z]+) "); // regexp syntax ending with (
  parser-> PartialMatch(sql, &this->sql_table_name);
  parser->Replace("", &sql);

  //----------- Getting the Collumns
  parser = new pcrecpp::RE("^(.*) VALUES "); //regexp syntax ending with VALUE
  parser->PartialMatch(sql, &this->sql_columns);
  //parser->Replace("",&sql);

  //--------- Getting the Values
  //this->sql_value_exp;

  //------ Debug Info
  this->print_debug_info();
}


//-----------------------------------------------
// ------ PARSE DELETE COMMAND ------------------
// ---------------------------------------------
void SqlInfo::parse_delete_command(string sql) {
  pcrecpp:: RE *parser;

  pcrecpp:: RE("(FROM )?").Replace("", &sql);

  // ---------- Geting the table name
  parser = new pcrecpp:: RE("^([_A-Za-z]+) WHERE");
  parser->PartialMatch(sql,&this->sql_table_name);
  parser->Replace("",&sql);

  // ---------- Getting the WHERE arguments
  this->sql_where_exp =  remove_values_from_where(sql);

 // ------- Debug info
 this->print_debug_info();

}

//-----------------------------------------------
// ------ PRINT DEBUG INFO  ---------------------
// ----------------------------------------------
void SqlInfo::print_debug_info() {
  if(loglevel == LOG_DEBUG){
     cout << "***********************************************************\n";
     cout << "* Warning: A SQL GET state change was detected...      \n";
     cout << "* QUERY TYPE:     " << this->sql_command_type << "\n";
     cout << "* TABLE NAME:     " << this->sql_table_name << "\n";
     cout << "* WHERE EXP:      " << this->sql_where_exp << "\n";
     cout << "***********************************************************\n";
  }
}

 //----- Remove all values and ',' from the field list
string SqlInfo::remove_values_from_update_fields_list(string fields_list) {
  pcrecpp::RE *parser;
  fields_list += ",";
  parser = new pcrecpp::RE("= ([^,]+),");
  parser->GlobalReplace("", &fields_list);
  return fields_list;
}


// ----- Remove values from where
string SqlInfo::remove_values_from_where(string where_field) {
  pcrecpp::RE(" ('[^']*') ").GlobalReplace("", &where_field);
  pcrecpp::RE(" ('[^']*')$").GlobalReplace("", &where_field);
  pcrecpp::RE(" (\"[^\"]*\") ").GlobalReplace("", &where_field);
  pcrecpp::RE(" ([0-9]+) ").GlobalReplace("", &where_field);
  return where_field;
}


