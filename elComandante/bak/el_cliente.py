# elCliente: Base class for all elComandante clients
#
# The class presents an interface to elComandante that all possible
# supervised clients (elCliente) have to implement.

class elCliente():
	def __init__(log, subscription):
		self.log = log
		self.subscription = subscription
		self.active = 0
		self.pending = 0
	def setup_configuration(conf):
		return True
	def setup_initialisation(init):
		return True
	def check_logfiles_presence():
		# Returns a list of logfiles present in the filesystem
		return []
	def check_client_running():
		return False
	def start_client():
		return False
	def check_subscription():
		return False
	def request_client_exit():
		return False
	def kill_client():
		return False
	def prepare_test():
		# Run before a test is executed
		return False
	def execute_test():
		# Runs a test
		return False
	def cleanup_test():
		# Run after a test has executed
		return False
