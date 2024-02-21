
class i2c_handler:
    def __init__(self, i2c):
        self._i2c_if = i2c

    def __enter__(self):
        while not self._i2c_if.try_lock():
            pass

        return self._i2c_if
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self._i2c_if.unlock()

    