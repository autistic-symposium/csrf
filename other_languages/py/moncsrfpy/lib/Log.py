class Log:
  def __init__(self, log_name):
    self.read_logfile(log_name)

  def read_logfile(self, log_name):
    self.__log_entries = array()
    with open('log_name', 'r+') as f:
      for line in f:
        self.__log_entries.append(line)

  def total_entries(self)
    return self.__log_entries.cout()

  def get_entry(self, entry)
    return self.__log_entries[entry]


