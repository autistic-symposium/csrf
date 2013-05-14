module LogParser
  # This class represents the log file to be parsed.
  # It will read all the entries of this file.
  class Log

    # Initializes the class.
    # Get a file pointer (log) as a parameter.
    # Read this file and parses it inside the object.
    def initialize(log_file, verbose = true)
      @verbose     = verbose
      @log_file    = log_file
      @log_entries = Array.new
      read_file
    end

    # Returns the entry from the log to the asked line.
    def get_entry(line)
      @log_entries[line]
    end

    # Returns the total number of entries from the log.
    def total_entries
      @log_entries.size
    end

    private

    # Parse log into log entries.
    def read_file
      puts 'MonCSRF will first read the file and then generates alerts. ' if @verbose
      print 'Reading the log file, it may take a while. ' if @verbose
      @log_entries = Array.new
      line_sum  = ""
      index     = 0
      last_in   = nil
      @log_file.each do |line|

        # There are entries in the log that ocuppy more than one line,
        # To fix this we look ahead here, detecting them and
        # saving in a buffer (line_sum).
        if multiline_entry?(look_ahead_in_log(index))
          line_sum += line
        else
          if line_sum.empty?
            @log_entries << LogParser::LogEntry.new(line)
          else
            @log_entries << LogParser::LogEntry.new((line_sum + line).gsub(/\n/, ''))
            line_sum = ""
          end
          # If the command is IN and for GET/PUT/POST, it will be an origin for
          # other entries.
          if @log_entries.last.io_type == :in && [:GET, :POST, :PUT].include?(@log_entries.last.command_type)
            last_in = @log_entries.last
          else
            # If it is OUT, we set the parent_command to the last_in of the buffer.
            @log_entries.last.parent_command = last_in
          end
        end
        print '.' if @verbose
        index += 1
      end
      puts 'Done!' if @verbose
    end

    # Because the log is not 1 to 1 for the lines, we need to verify them,
    # i.e., there are entries with more than one line,
    # We return true if this is the case.
    def multiline_entry?(line)
      return true unless line
      if line.match(/^(in|out)\s/)
        false
      else
        true
      end
    end

    # Looks to the next line to see if the command has more than one line,
    def look_ahead_in_log(line_number)
      begin
        internal_file_pointer = File.new(@log_file.path)
        line = internal_file_pointer.readlines[line_number+1]
        line
      rescue
        nil
      end
    end


  end
end
