import re

class SqlInfo:
  def __init__(self, sql):
    self.__sql_command = self.__normalize_sql(sql)
    self.__parse()

  def compare(self, other_sql_info):
    if self.get_sql_command_type() == other_sql_info.get_sql_command_type():
      if self.get_sql_table_name() == other_sql_info.get_sql_table_name():
        if self.get_sql_columns() == other_sql_info.get_sql_columns():
          if self.get_sql_where_exp() == other_sql_info.get_sql_where_exp():
            return True

  def get_sql_command(self):
    return self.__sql_command

  def get_sql_command_type(self):
    return self.__sql_command_type

  def get_sql_table_name(self):
    return self.__sql_table_name

  def get_sql_columns(self):
    return self.__sql_columns

  def get_sql_where_exp(self):
    return self.__sql_where_exp

  def __parse(self):
    match = re.search("^[A-Za-z]+\s", self.__sql_command)
    self.__sql_command_type = match.group(0).strip()
    sql   = re.sub("^[A-Za-z]+\s", "", self.__sql_command)

    if    self.__sql_command_type == 'UPDATE':
      self.__parse_update_command(sql)
    elif  self.__sql_command_type == 'INSERT':
      self.__parse_insert_command(sql)
    elif  self.__sql_command_type == 'DELETE':
      self.__parse_delete_command(sql)


  def __parse_update_command(self, sql):
    match = re.search("^[_A-Za-z]+\s", sql)
    self.__sql_table_name = match.group(0).strip()

    sql = re.sub("^[_A-Za-z]+\sSET", "", sql)
    sql = sql.split('WHERE')

    self.__sql_columns = re.sub("\s?=\s?[^,=]*\s?(,|$)", "", sql[0].strip())
    self.__sql_where_exp = self.__filter_where_exp(sql[1].strip())

    return True

  def __parse_insert_command(self, sql):
    sql = re.sub("(INTO\s)?", "", sql).strip()      # Remove INTO se tiver
    match = re.search("^[_A-Za-z]+\s",sql)          # Nome da tabela
    self.__sql_table_name = match.group(0).strip()
    sql = re.sub("^[_A-Za-z]+\s", "", sql)          # Remove o nome da tabela da string

    sql = sql.split('VALUES') # Quebra a string em um array com o q vem antes e depois de VALUES
    sql = sql[0].strip()      # O primeiro elemento tem as colunas
    sql = re.sub("^\(", "", sql) # Remove o primeiro (
    sql = re.sub("\)$", "", sql) # Remove o ultimo (
    self.__sql_columns = sql
    self.__sql_where_exp = ""
    return True


  def __parse_delete_command(self, sql):
    sql = re.sub("(FROM\s)?","",sql).strip()
    match = re.search("^[_A-Za-z]+\s",sql)          # Nome da tabela
    self.__sql_table_name = match.group(0).strip()
    sql = re.sub("^[_A-Za-z]+\s", "", sql)          # Remove o nome da tabela da string

    sql = sql.split('WHERE')

    self.__sql_columns = re.sub("\s?=\s?[^,=]*\s?(,|$)", "", sql[0].strip())
    self.__sql_where_exp = self.__filter_where_exp(sql[1].strip())

    return True

  def __filter_where_exp(self, sql):
    sql = re.sub("\s?'[^']*'\s?AND","AND",sql)
    sql = re.sub("\s?\"[^\"]*\"\s?AND","AND",sql)
    sql = re.sub("\s?[0-9]*\s?AND","AND",sql)

    sql = re.sub("\s?'[^']*'\s?OR","OR",sql)
    sql = re.sub("\s?\"[^\"]*\"\s?OR","OR",sql)
    sql = re.sub("\s?[0-9]*\s?OR","OR",sql)

    sql = re.sub("\s?'[^']*'\s?$","",sql)
    sql = re.sub("\s?\"[^\"]*\"\s?$","",sql)
    sql = re.sub("\s?[0-9]*\s?$","",sql)
    return sql


  def __normalize_sql(self, sql):
    sql = re.sub("\n", " ", sql)
    sql = re.sub("\t", " ", sql)
    sql = re.sub("( )+", " ", sql)
    return sql
