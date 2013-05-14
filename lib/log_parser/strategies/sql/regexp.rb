# This class is a logiv for a parse based on regular expressions (deprecated).

module LogParser
  module Strategies
    module Sql
      class Regexp < Base

        private

        def parse sql
          super

          # Selects only insert/update/delete requests.
          type = sql.match(/^(INSERT|UPDATE|DELETE)\s/i)
          return unless type

          # Saves the type of the query.
          @sql_type = type[1].downcase.to_sym

          # Removes the query from the SQL string.
          sql.gsub!(/^[A-Za-z]+\s+/, "")

          case @sql_type
          when :update
            parse_update(sql)
          when :delete
            parse_delete(sql)
          when :insert
            parse_insert(sql)
          end
        end

        def parse_update sql
          # Gets the table_name and removes it from the string.
          @table_name = sql.match(/^([_A-Za-z]*)\s+/)[1].to_sym
          sql.gsub! /^[_A-Za-z]*\s+(SET\s+)?/, ""

          # Divides the string into an array.
          # 0 -> columns
          # 1 -> where
          sql.gsub! /\sWHERE\s/i, "where"
          sql = sql.split 'where'

          # Treating the collumns.
          @columns = Array.new
          sql[0].split(',').each do |column|
            @columns << column.gsub(/(\s+)?=.*/,"").strip.to_sym
          end
          @columns.sort!

          # Treating the WHERE requests.
          @where = parse_where(sql[1])
        end

        def parse_insert sql
          # Gets the table name and removes it from the string.
          @table_name = sql.match(/^(INTO\s+)?\s*([_A-Za-z]*)/i)[2].to_sym
          sql.gsub!(/^(INTO\s+)?\s*([_A-Za-z]*)/i, '')

          # Divides the collum into array.
          # 0 -> columns
          # 1 -> values

          sql.gsub! /\sVALUES\s/i, '_VALUES_'
          sql = sql.split '_VALUES_'

          # Treating collumns.
          @columns = Array.new
          sql[0].gsub! /\(|\)/, ''
          sql[0].split(',').each do |column|
            @columns << column.strip.to_sym
          end
          @columns.sort!
        end

        def parse_delete sql
          # Gets the table name and remove from the string.
          @table_name = sql.match(/^(FROM\s+)?\s*([_A-Za-z]*)/i)[2].to_sym
          sql.gsub!(/^(FROM\s+)?\s*([_A-Za-z]*)\s*WHERE\s*/i, '')

          # Gets the WHERE requests.
          @where = parse_where(sql.strip)
        end

        def parse_where(sql)
          sql.gsub!(/\s\'[^']*\'(\s|$)/ , "")
          sql.gsub!(/\s\"[^"]*\"(\s|$)/ , "")
          sql.gsub!(/\s[0-9]*(\s|$)/    , "")
          sql.gsub(/\s+/                , "")
        end

      end
    end
  end
end
