import unittest
from lib.Log import Log

class LogTestCase(unittest.TestCase):

  # Ve se o arquivo foi parseado na quantidade certa de linhas
  def test_initialization(self):
    log = Log(self.log_name())
    #self.assertEqual(log.total_entries(), self.log_name().lines())
    # o .lines() eu nao sei, deve ter algum metodo q retorna o numero de linhas .size, sei la


if __name__ == '__main__':
    unittest.main()

