import unittest
from lib.sql_info import SqlInfo

class SqlInfoTestCase(unittest.TestCase):

  def test_equal_update_comparisson(self):
    sql_info_1 = SqlInfo("UPDATE phpbb_sessions\
                  SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '1'\
                          AND session_ip = '7f000001'")
    sql_info_2 = SqlInfo("UPDATE phpbb_sessions\
                  SET session_user_id = -1, session_start = 213124, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '5'\
                          AND session_ip = '7f000001'")

    self.assertTrue(sql_info_1.compare(sql_info_2))

  def test_different_update_comparisson(self):
    sql_info_1 = SqlInfo("UPDATE phpbb_sessions\
                  SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '1'\
                          AND session_ip = '7f000001'")
    sql_info_2 = SqlInfo("UPDATE phpbb_sessions\
                  SET user = -1, session_start = 213124, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '5'\
                          AND session_ip = '7f000001'")
    self.assertFalse(sql_info_1.compare(sql_info_2))


  def test_equal_insert_comparisson(self):
    sql_info_1 = SqlInfo("INSERT INTO users (name, email) VALUES ('joao', 'joao@bola.com')")
    sql_info_2 = SqlInfo("INSERT INTO users (name, email) VALUES ('maria', 'maria@bola.com')")
    self.assertTrue(sql_info_1.compare(sql_info_2))

  def test_different_insert_comparisson(self):
    sql_info_1 = SqlInfo("INSERT INTO users (name, email) VALUES ('joao', 'joao@bola.com')")
    sql_info_2 = SqlInfo("INSERT INTO cool_users (name, email) VALUES ('maria', 'maria@bola.com')")
    self.assertFalse(sql_info_1.compare(sql_info_2))

  def test_equal_delete_comparisson(self):
    sql_info_1 = SqlInfo("DELETE FROM users WHERE id = 1")
    sql_info_2 = SqlInfo("DELETE FROM users WHERE id = 5")
    self.assertTrue(sql_info_1.compare(sql_info_2))

  def test_different_delete_comparisson(self):
    sql_info_1 = SqlInfo("DELETE FROM users WHERE id = 1")
    sql_info_2 = SqlInfo("DELETE FROM cool_users WHERE id = 5")
    self.assertFalse(sql_info_1.compare(sql_info_2))

  def test_different_types_comparisson(self):
    sql_info_1 = SqlInfo("INSERT INTO users (name, email) VALUES ('joao', 'joao@bola.com')")
    sql_info_2 = SqlInfo("DELETE FROM users WHERE id = 5")
    self.assertFalse(sql_info_1.compare(sql_info_2))

  def test_normalizing_sql(self):
    sql_info_1 = SqlInfo("UPDATE phpbb_sessions\
                  SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '1'\
                          AND session_ip = '7f000001'")
    self.assertEqual(sql_info_1.get_sql_command(),"UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'")


  def test_update_parsing(self):
    sql_info_1 = SqlInfo("UPDATE phpbb_sessions\
                  SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0\
                  WHERE session_id = '1'\
                          AND session_ip = '7f000001'")
    self.assertEqual(sql_info_1.get_sql_command_type() , 'UPDATE')
    self.assertEqual(sql_info_1.get_sql_table_name()   , 'phpbb_sessions')
    self.assertEqual(sql_info_1.get_sql_columns()      , 'session_user_id session_start session_time session_page session_logged_in')
    self.assertEqual(sql_info_1.get_sql_where_exp()    , 'session_id =AND session_ip =')

  def test_delete_parsing(self):
    sql_info_1 = SqlInfo("DELETE FROM users WHERE id = 1")
    self.assertEqual(sql_info_1.get_sql_command_type() , 'DELETE')
    self.assertEqual(sql_info_1.get_sql_table_name()   , 'users')
    self.assertEqual(sql_info_1.get_sql_columns()      , '')
    self.assertEqual(sql_info_1.get_sql_where_exp()    , 'id =')

  def test_insert_parsing(self):
    sql_info_1 = SqlInfo("INSERT INTO users (name, email) VALUES ('joao', 'joao@bola.com')")
    self.assertEqual(sql_info_1.get_sql_command_type() , 'INSERT')
    self.assertEqual(sql_info_1.get_sql_table_name()   , 'users')
    self.assertEqual(sql_info_1.get_sql_columns()      , 'name, email')
    self.assertEqual(sql_info_1.get_sql_where_exp()    , '')


if __name__ == '__main__':
    unittest.main()
