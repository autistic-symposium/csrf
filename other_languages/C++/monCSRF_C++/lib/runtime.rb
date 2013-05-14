# When we run /bin/parser.rb this is the main class that will be called to run the parser.
# For each line from the log file, it searches entries that are modifying the state of the system.
# It loops over the entries searching SQL requests that matches these conditions.
# If it finds a match, it prints an warning message.
class Runtime

  # Initiates the parser.
  def self.run(log_file_name, benchmark = false)
    verbose = false
    verbose = true unless benchmark

    # Opens the log file.
    log = LogParser::Log.new(File.open(log_file_name), verbose)

    # Opens the whitelist file.
    white_list = LogParser::WhiteList.new('white_list')

    # Loops in all the entries of the log.
    log.total_entries.times.each do |line|

      entry = log.get_entry(line)
      exit if benchmark

      # If the command is SQL:
      if entry.command_type == :mysql_query
        sql_info = LogParser::SqlInfo.new(entry.parameter 0)

        # If the SQL changed the state of the system,
        # (It might be an attack).
        if [:update, :insert, :delete].include?(sql_info.sql_type)

          puts "              ------------------------------------------------"
          puts "[WARNING]: A query \"#{sql_info.normalized_sql}\" was requested."
          puts "           The origin request was: #{entry.parent_command.original_line}"

          # If this request was not in the whitelist, the program flags it.
          unless white_list.listed?(sql_info)
            puts "               -------------------------------------------"
            puts "[ACTION REQUIRED]: GET request changing the system state!" if entry.command_type == :GET
            puts "[ACTION REQUIRED]: POST request changing the system state!" if entry.command_type == :POST
            puts "Do you want to flag this request as safe? (y/n)"
            STDOUT.flush
            white_list_it = STDIN.gets.chomp

            # If we mark the previous request to be whitelisted.
            if ['yes', 'y'].include?(white_list_it.downcase)
              white_list.add!(sql_info)
              puts "           [y] The request was whitelisted."
            else
              puts "           [n] The request was NOT whitelisted."
            end

          else
            puts "[ATTENTION]: Similar request was already in the whitelist."
            puts "              ------------------------------------------------"
          end
        end
      end

    end

  end
end
