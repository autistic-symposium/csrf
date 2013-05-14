module LogParser
  # This class represents one SQL query.
  #Type, collumn, where, etc...
  #It accepts strategies for the parses.

  class SqlInfo < LogParser::Strategies::Sql::Regexp
  #class SqlInfo < LogParser::Strategies::Sql::Ragel

    def initialize(sql)
      parse(sql)
    end

  end
end
