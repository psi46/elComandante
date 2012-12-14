class xray_generator():
	def __init__(self, number_of_beams):
		self.number_of_beams = number_of_beams
	def is_open(self):
		return False
	def test_communication(self):
		return False
	def set_voltage(self, kV):
		return None
	def set_current(self, mA):
		return None
	def set_hv(self, on):
		return None
	def set_beam_shutter(self, beamno, on):
		return self.set_hv(on)
