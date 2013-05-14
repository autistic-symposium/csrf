module LogParser
  module Strategies
    module Sql
      class Ragel < Base

        %%{

          machine ragel;

          action str_a {
            str_buf += fc.chr
          }

          action value {
            last_field_buf = str_buf
            str_buf = ""
          }

          action literal {
            str_buf = ""
          }

          action operator {
            last_operator = str_buf
            str_buf = ""
          }

          action logical_operator {
            object_store.where += str_buf
            str_buf = ""
          }

          action table_name {
            object_store.table_name = str_buf.to_sym
            str_buf = ""
          }

          action string {
          }

          action dstring {
          }

          action column_store {
            object_store.columns = Array.new unless object_store.columns.is_a? Array
            object_store.columns << str_buf.to_sym
            object_store.columns.sort!
            str_buf = ""
          }

          action where_exp {
          }

          action left_value {
            object_store.where = "" unless object_store.where
            object_store.where += last_field_buf + last_operator
          }

          and      = /AND/i $str_a;
          or       = /OR/i $str_a;

          ws       = ' ' | '\t' | '\n';
          var      = ws* ([a-zA-Z_][a-zA-Z0-9_:]*) $str_a;
          field    = var;
          is_null  = ws+ 'is' ws+ 'null' $str_a;

          real     = ws* ('-'? ('0'..'9' digit* '.' digit+) ) $str_a;
          integer  = ws* ('-'? digit+ ) $str_a;
          number   = (real | integer);
          dquote   = ([^"\\] | '\n') $str_a | ('\\' (any | '\n') $str_a);
          squote   = ([^'\\] | '\n') $str_a | ('\\' (any | '\n') $str_a);
          string   = ws* ("'" squote* "'" >string | '"' dquote* '"' >dstring);
          literal  = number;

          value             = (field | literal | string) % value;
          logical_operators = (and | or) % logical_operator;
          logical_operator  = (logical_operators);
          arit_operators    = ('>' | '<' | '<=' | '>=' | '=') $str_a;
          arit_operator     = arit_operators % operator;

          left_value = value ws* arit_operator % left_value;
          expression = left_value ws* value;
          table_name = ws+ var % table_name;

          SELECT = /SELECT/i @ { object_store.sql_type = :select };
          UPDATE = /UPDATE/i @ { object_store.sql_type = :update };
          DELETE = /DELETE/i @ { object_store.sql_type = :delete };
          INSERT = /INSERT/i @ { object_store.sql_type = :insert };
          FROM   = /FROM/i;
          WHERE  = /WHERE/i;
          SET    = /SET/i;
          INTO   = /INTO/i;
          VALUES = /VALUES/i;

          where_exp     = (ws+ expression (ws+ logical_operator ws+ expression)*) % where_exp;
          where_stmt    = ws+ WHERE where_exp;

          column_store  = field % column_store;
          select_fields = column_store (ws* ',' ws* column_store)*;
          select_stmt   = SELECT ws+ select_fields ws+ FROM table_name where_stmt? ws* ';';

          update_field  = ws* column_store ws* '=' value;
          update_fields = update_field (ws* ',' ws* update_field)*;
          update_stmt   = UPDATE table_name ws+ SET update_fields where_stmt? ws* ';';

          delete_stmt = DELETE (ws+ FROM)? table_name where_stmt? ws* ';';

          values_fields = value (ws* ',' ws* value)*;
          insert_stmt = INSERT ws+ INTO table_name ws+ '(' ws* select_fields ws* ')' ws+ VALUES ws+ '(' values_fields ')' ws* ';';

          main := select_stmt | delete_stmt | update_stmt | insert_stmt;

        }%%

        %% write data;

        def self.parse(data, object_store)
          str_buf = ""
          last_field_buf = nil
          last_operator  = nil
          expression_buf = ""

          data = data.unpack("c*") if data.is_a?(String)

          %% write init;
          %% write exec;

        end

        private

        def parse(sql)
          super
          Ragel.parse(sql, self)
        end

      end
    end
  end
end

