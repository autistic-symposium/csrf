# This class is the base to any of the SQL strategies we decide to use,
# therefore, every SQL strategy should containt it.

module LogParser
  module Strategies
    module Sql
      class Base

        attr_accessor :original_sql
        attr_accessor :normalized_sql
        attr_accessor :sql_type
        attr_accessor :table_name
        attr_accessor :columns
        attr_accessor :where

        # Defines the operator == to compare two SQLs.
        def ==(other)
          if @sql_type == other.sql_type
            if @table_name == other.table_name
              if @columns == other.columns
                if @where == other.where
                  return true
                end
              end
            end
          end
          return false
        end

        private

        def parse(sql)
          @original_sql   = sql.dup
          @normalized_sql = normalize_sql(sql.dup)
        end

        def normalize_sql(sql)
          sql.gsub!(/\n/  , " ")
          sql.gsub!(/\r/  , " ")
          sql.gsub!(/\t/  , " ")
          sql.gsub!(/\s+/ , " ")
        end
      end
    end
  end
end
