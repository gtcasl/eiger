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
		self.assertFalse(self.dc.machines)
		self.assertFalse(self.dc.applications)
		self.assertFalse(self.dc.trials)
		self.assertFalse(self.dc.datasets)
		self.assertFalse(self.dc.metrics)




if __name__ == "__main__":
	unittest.main()

