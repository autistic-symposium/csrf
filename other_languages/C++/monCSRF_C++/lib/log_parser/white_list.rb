# This class verifies if the new request is in the whitelist.
# If it is not, it saves the safe entries into the file.

module LogParser
  class WhiteList

    attr_accessor :entries

    def initialize(white_list_filename)
      @white_list_filename = white_list_filename
      parse_white_list
    end

    # Verififies if a SqlInfo is in the whitelist.
    def listed?(entry)
      @entries.each do |white_entry|
        return true if entry == white_entry
      end
      return false
    end

    def add!(entry)
      @entries << entry
      write_new_entry(entry.normalized_sql)
    end

    private

    def write_new_entry(normalized_sql)
      file = File.open(@white_list_filename, 'a') do |f|
        f.puts(normalized_sql)
      end
    end

    # Parses from the whitelist into an array of SqlInfos.
    def parse_white_list
      @entries = Array.new
      begin
        File.open(@white_list_filename).each do |line|
          @entries << LogParser::SqlInfo.new(line)
        end
      rescue
        File.new(@white_list_filename, 'w')
      end
    end

  end
end
