module LogParser
  # This class represents one entry from the log file.
  # It will look to each part of the request:
  #  -> GET, POST, etc.,
  #  -> type IN/OUT,
  #  -> parameters.
  class LogEntry

    # Stores the original command.
    attr_accessor :parent_command

    def initialize(line)
      @original_line = line
      parse_line line.dup
    end

    # Stores the original line.
    def original_line
      @original_line
    end

    # Gets if it is type IO (IN / OUT).
    def io_type
      @line_io_type.to_sym
    end

    # Gets the command type (GET, POST, mysql_query, ...).
    def command_type
      @line_command_type.to_sym
    end

    # Gets the total number of parameters.
    def total_parameters
      @line_parameters.size
    end

    # Gets the number of parameter as an argument.
    def parameter(param_number = 0)
      @line_parameters[param_number]
    end

    private

    # Parses the line.
    def parse_line(line)
      # IO
      @line_io_type = line.match(/^(in|out)\s/) {|match| match[1] }  # getting the kind of IO (IO or OUT)
      line.gsub!(/^(in|out)\s/, "")                                  # removing it from the line.

      # TYPE
      @line_command_type = line.match(/^([\_A-Za-z]*)\s/) {|match| match[1] }
      line.gsub!(/^([_A-Za-z]*)\s/, "")

      # PARAMETRS
      total_params = line.match(/^([0-9]*)\s/) {|match| match[1].to_i }
      line.gsub!(/^([0-9]*)\s+/, "")       # Remove the number of params from the line.

      matches = line.match(create_reg_exp_for_n_params(total_params))

      @line_parameters = Array.new
      total_params.times do |i|
        @line_parameters[i] = matches[i+1]
      end

    end

    # Creates a REGEXP to the number of parameters.
    def create_reg_exp_for_n_params(total_params)
      begin
      spaces = "[0-9]+\s+" * total_params
      values = "(.*)\s" * (total_params -1)
      values += "(.*)$"
      "#{spaces}#{values}"
      rescue
        puts "TOO MANY PARAMETERS!!!"
        puts @original_line
        puts total_params
      end
    end

  end
end
