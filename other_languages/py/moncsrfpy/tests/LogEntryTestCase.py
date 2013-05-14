import unittest
from lib.LogEntry import LogEntry

class LogEntryTestCase(unittest.TestCase):

  def test_in_get_entry_parse(self):
    entry = LogEntry("in GET 1     139 /phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest")
    self.assertEqual(entry.get_io(), "in")
    self.assertEqual(entry.get_type(), "GET")
    self.assertEqual(entry.get_value(), "/phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest")

  def test_in_param_entry_parse(self):
    entry = LogEntry("in parameter 2       8      84 topic_id -1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*")
    self.assertEqual(entry.get_io(), "in")
    self.assertEqual(entry.get_type(), "parameter")
    self.assertEqual(entry.get_param(), "topic_id")
    self.assertEqual(entry.get_value(), "-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*")

  def test_out_mysql_entry_parse(self):
    entry = LogEntry("out mysql_query 1      27 SELECT *\
	FROM phpbb_config")
    self.assertEqual(entry.get_io(), "out")
    self.assertEqual(entry.get_type(), "mysql_query")
    self.assertEqual(entry.get_value(), "SELECT *\
	FROM phpbb_config")

  def test_out_mysql_entry_parse(self):
    entry = LogEntry("out fopen 1      37 ./language/lang_english/lang_main.php")
    self.assertEqual(entry.get_io(), "out")
    self.assertEqual(entry.get_type(), "fopen")
    self.assertEqual(entry.get_value(), "./language/lang_english/lang_main.php")








