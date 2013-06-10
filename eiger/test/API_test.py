import unittest
from API import DataCollection

class TestDataCollection(unittest.TestCase):
	"""
	Each test method needs "test" in the beginning of the name.

		def test_one(self):
			self.assertEqual(1,2)

	"""
	def setUp(self):
		self.dc = DataCollection()

	def testEmpty(self):
		self.assertTrue(self.dc.machines.empty())
		self.assertTrue(self.dc.applications.empty())
		self.assertTrue(self.dc.trials.empty())
		self.assertTrue(self.dc.datasets.empty())
		self.assertTrue(self.dc.metrics.empty())




if __name__ == "__main__":
	unittest.main()

