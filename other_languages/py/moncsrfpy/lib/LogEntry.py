import re

class LogEntry:
  def __init__(self, entry):
    self.__original_entry = entry
    self.__parse_entry(entry)

  def get_type(self):
    return self.__log_type

  def get_io(self):
    return self.__log_io

  def get_value(self):
    return self.__log_value

  def get_param(self):
    return self.__log_param

  def __parse_entry(self, entry):
    match = re.search("^([^\s]*)\s", entry)
    self.__log_io = match.group(0).strip()
    entry = re.sub("^([^\s]*)\s", "", entry)

    match = re.search("^([^\s]*)\s", entry)
    self.__log_type = match.group(0).strip()
    entry = re.sub("^([^\s]*)\s", "", entry)

    items_match = 0
    request_items = entry.split(" ")

    for request_item in request_items:
      print "----> " + request_item
      if re.search("^[0-9]+", request_item):
        request_items.remove(request_item)
      else:
        items_match += 1
        if (self.__log_io == "in") and (self.__log_type == "parameter"):
          if (items_match == 1):
            self.__log_param = request_item
            request_items.remove(request_item)
            continue
        self.__log_value = request_items.join(' ')
        break

    return True

